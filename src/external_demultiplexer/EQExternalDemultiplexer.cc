#include "HPNL/EQExternalDemultiplexer.h"

EQExternalDemultiplexer::EQExternalDemultiplexer(FIStack *stack_) : stack(stack_) {
  fabric = stack->get_fabric();
  epfd = epoll_create1(0);
  memset((void*)&event, 0, sizeof event);
}

EQExternalDemultiplexer::~EQExternalDemultiplexer() {
  fid_map.clear();
  close(epfd);
}

int EQExternalDemultiplexer::wait_event(fi_info** info, fid_eq** eq) {
  struct fid *fids[fid_map.size()];
  int i = 0;
  for (auto iter: fid_map) {
    fids[i++] = iter.first;
  }
  if (fi_trywait(fabric, fids, fid_map.size()) == FI_SUCCESS) {
    int epoll_ret = epoll_wait(epfd, &event, 1, 200);
    if (epoll_ret <= 0) {
      return epoll_ret;
    }
    if (fid_map.count((fid*)event.data.ptr) == 0) {
      std::cout << "got error event" << std::endl;
      return -1;
    }
    *eq = fid_map[(fid*)event.data.ptr];
  }

  if (*eq == NULL) {
    return -1;
  }

  int ret = 0;
  uint32_t event;
  fi_eq_cm_entry entry;
  ret = fi_eq_read(*eq, &event, &entry, sizeof(entry), 0);
  if (ret == -FI_EAGAIN) {
    return 0; 
  } else if (ret < 0) {
    fi_eq_err_entry err_entry;
    fi_eq_readerr(*eq, &err_entry, event);
    return -1;
  } else {
    entry.fid = &(*eq)->fid;
    if (event == FI_CONNREQ) {
      *info = entry.info;
      return ACCEPT_EVENT;
    } else if (event == FI_CONNECTED)  {
      auto con = stack->get_connection(entry.fid);
      assert(con);
      con->init_addr();
      return CONNECTED_EVENT;
    } else if (event == FI_SHUTDOWN) {
      delete_event(*eq);
      auto con = stack->get_connection(entry.fid);
      if (con) {
        con->status = DOWN;
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
  if (fid_map.count(&eq->fid) != 0) return -1;
  int fd;
  int ret = fi_control(&eq->fid, FI_GETWAIT, (void*)&fd);
  if (ret) {
    std::cout << "fi_controll error." << std::endl;
  }
  event.events = EPOLLIN;
  event.data.ptr = &eq->fid;
  ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
  if (ret) {
    std::cout << "epoll add error." << std::endl;
  }
  fid_map.insert(std::make_pair(&eq->fid, eq));
  return 0;
}

int EQExternalDemultiplexer::delete_event(fid_eq *eq) {
  std::lock_guard<std::mutex> lk(mtx);
  if (fid_map.count(&eq->fid) == 0) return -1;
  int fd;
  int ret = fi_control(&eq->fid, FI_GETWAIT, (void*)&fd);
  if (ret) {
    std::cout << "fi_controll error." << std::endl;
  }
  event.events = EPOLLIN;
  event.data.ptr = &eq->fid;
  ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
  if (ret) {
    std::cout << "epoll delete error." << std::endl;
    return -1;
  }
  fid_map.erase(&eq->fid);
  return 0;
}
