#include "HPNL/EQExternalDemultiplexer.h"

#include "rdma/fi_errno.h"

EQExternalDemultiplexer::EQExternalDemultiplexer(FIStack *stack_) : stack(stack_) {}

EQExternalDemultiplexer::~EQExternalDemultiplexer() {
  fid_map.clear();
  close(epfd);
}

int EQExternalDemultiplexer::init() {
  fabric = stack->get_fabric();
  epfd = epoll_create1(0);
  if (epfd == -1) {
    perror("epoll_create1");
    return -1;
  }
  memset((void*)&event, 0, sizeof event);
  return 0;
}

int EQExternalDemultiplexer::wait_event(fi_info** info, fid_eq** eq, FIConnection** con) {
  struct fid *fids[fid_map.size()];
  int i = 0;
 
  for (auto iter: fid_map) {
    fids[i++] = iter.first;
  } 
  if (fi_trywait(fabric, fids, fid_map.size()) == FI_SUCCESS) {
    int epoll_ret = epoll_wait(epfd, &event, 1, 200);
    if (epoll_ret > 0) { 
      *eq = fid_map[(fid*)event.data.ptr]; 
    } else if (epoll_ret == -1) {
      if (errno != EINTR) {
        perror("epoll_wait");
        return -1;
      }
      return 0;
    } else {
      return 0; 
    }
  } 

  uint32_t event;
  fi_eq_cm_entry entry;
  int ret = fi_eq_read(*eq, &event, &entry, sizeof(entry), 0);
  if (ret == -FI_EAGAIN) {
    return 0; 
  } else if (ret < 0) {
    fi_eq_err_entry err_entry;
    fi_eq_readerr(*eq, &err_entry, event);
    perror("fi_eq_read");
    if (err_entry.err == FI_EOVERRUN) {
      return -1;
    }
    return 0;
  } else {
    entry.fid = &(*eq)->fid;
    if (event == FI_CONNREQ) {
      *info = entry.info;
      return ACCEPT_EVENT;
    } else if (event == FI_CONNECTED)  {
      *con = stack->get_connection(entry.fid);
      (*con)->init_addr();
      return CONNECTED_EVENT;
    } else if (event == FI_SHUTDOWN) {
      delete_event(*eq);
      *con = stack->get_connection(entry.fid);
      if (*con) {
        (*con)->status = DOWN;
        stack->reap(entry.fid);
      }
      return SHUTDOWN;
    } else {
      return 0;
    }
  }
  return 0;
}

int EQExternalDemultiplexer::add_event(fid_eq *eq) {
  std::lock_guard<std::mutex> lk(mtx);
  if (fid_map.count(&eq->fid) != 0) {
    std::cerr << "got unknown eq fd" << std::endl;
    return -1;
  }
  int fd;
  if (fi_control(&eq->fid, FI_GETWAIT, (void*)&fd)) {
    perror("fi_control");
    goto quit_add_event;
  }
  if(!fd){
	return -1;
  }
  event.events = EPOLLIN;
  event.data.ptr = &eq->fid;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0) {
    perror("epoll_ctl");
    goto quit_add_event;
  }
  fid_map.insert(std::make_pair(&eq->fid, eq));
  return 0;
quit_add_event:
  close(fd);
  return -1;
}

int EQExternalDemultiplexer::delete_event(fid_eq *eq) {
  std::lock_guard<std::mutex> lk(mtx);
  if (fid_map.count(&eq->fid) == 0) return -1;
  int fd;
  if (fi_control(&eq->fid, FI_GETWAIT, (void*)&fd)) {
    perror("fi_control");
    goto quit_delete_event;
  }
  event.events = EPOLLIN;
  event.data.ptr = &eq->fid;
  if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event) < 0) {
    perror("epoll_ctl");
    goto quit_delete_event;
  }
  fid_map.erase(&eq->fid);
  return 0;
quit_delete_event:
  close(fd);
  return -1;
}
