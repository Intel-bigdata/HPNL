#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/Handle.h"
#include "demultiplexer/EventHandler.h"
#include "core/FiStack.h"

#include <iostream>

EqDemultiplexer::EqDemultiplexer(FiStack *stack_) : stack(stack_) {}

EqDemultiplexer::~EqDemultiplexer() {}

int EqDemultiplexer::init() {
  fabric = stack->get_fabric();
  epfd = epoll_create1(0);
  if (epfd == -1) {
    perror("epoll_create1");
    return -1;
  }
  memset((void*)&event, 0, sizeof event);
  return 0;
}

int EqDemultiplexer::wait_event(std::map<std::shared_ptr<Handle>, std::shared_ptr<EventHandler>> eventMap) {
  if (fid_map.empty()) return 0;
  struct fid *fids[fid_map.size()];
  int i = 0;
  for (auto iter: fid_map) {
    fids[i++] = iter.first;
  }
  std::shared_ptr<Handle> handlePtr;
  if (fi_trywait(fabric, fids, fid_map.size()) == FI_SUCCESS) {
    int epoll_ret = epoll_wait(epfd, &event, 1, 200);
    if (epoll_ret > 0) {
      handlePtr = fid_map[(fid*)event.data.ptr];
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
  int ret = fi_eq_read((fid_eq*)handlePtr->get_ctx(), &event, &entry, sizeof(entry), 2000);
  if (ret == -FI_EAGAIN) {
    return 0; 
  } else if (ret < 0) {
    fi_eq_err_entry err_entry;
    fi_eq_readerr((fid_eq*)handlePtr->get_ctx(), &err_entry, event);
    return 0; 
  } else {
    entry.fid = handlePtr->get_fid();
    if (event == FI_CONNREQ) {
      eventMap[handlePtr]->handle_event(ACCEPT_EVENT, &entry); 
    } else if (event == FI_CONNECTED)  {
      eventMap[handlePtr]->handle_event(CONNECTED_EVENT, &entry);
    } else if (event == FI_SHUTDOWN) {
      eventMap[handlePtr]->handle_event(CLOSE_EVENT, &entry); 
    } else {
    }
  }
  return 0;
}

int EqDemultiplexer::register_event(std::shared_ptr<Handle> handlePtr) {
  std::lock_guard<std::mutex> lk(mtx);
  if (fid_map.count(&((fid_eq*)handlePtr->get_ctx())->fid) != 0) {
    std::cerr << __func__ << " got unknown eq fd" << std::endl;
    return -1;
  }
  int fd;
  if (fi_control(&((fid_eq*)handlePtr->get_ctx())->fid, FI_GETWAIT, (void*)&fd)) {
    perror("fi_control");
    goto quit_add_event;
  }
  event.events = EPOLLIN;
  event.data.ptr = &((fid_eq*)handlePtr->get_ctx())->fid;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0) {
    perror("epoll_ctl");
    goto quit_add_event;
  }
  fid_map.insert(std::make_pair(&((fid_eq*)handlePtr->get_ctx())->fid, handlePtr));
  return 0;
quit_add_event:
  close(fd);
  return -1;
}

int EqDemultiplexer::remove_event(std::shared_ptr<Handle> handlePtr) {
  std::lock_guard<std::mutex> lk(mtx);
  if (fid_map.count(&((fid_eq*)handlePtr->get_ctx())->fid) == 0) {
    std::cerr << __func__ << "_got unknown eq fd" << std::endl;
    return -1;
  }
  int fd;
  if (fi_control(&((fid_eq*)handlePtr->get_ctx())->fid, FI_GETWAIT, (void*)&fd)) {
    perror("fi_control");
    goto quit_delete_event;
  }
  event.events = EPOLLIN;
  event.data.ptr = &((fid_eq*)handlePtr->get_ctx())->fid;
  if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event) < 0) {
    perror("epoll_ctl");
    goto quit_delete_event;
  }
  fid_map.erase(&((fid_eq*)handlePtr->get_ctx())->fid);
  return 0;
quit_delete_event:
  close(fd);
  return -1;
}

void EqDemultiplexer::shutdown() {
}
