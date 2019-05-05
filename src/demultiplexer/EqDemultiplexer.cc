#include <iostream>

#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/EventHandler.h"
#include "core/FiStack.h"

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

int EqDemultiplexer::wait_event(std::map<fid*, std::shared_ptr<EventHandler>> event_map) {
  if (event_map.empty()) return 0;
  struct fid *fids[event_map.size()];
  int i = 0;
  for (auto iter: event_map) {
    fids[i++] = iter.first;
  }
  fid_eq *eq;
  if (fi_trywait(fabric, fids, event_map.size()) == FI_SUCCESS) {
    int epoll_ret = epoll_wait(epfd, &event, 1, 200);
    if (epoll_ret > 0) {
      eq = event_map[(fid*)event.data.ptr]->get_handle();
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
  int ret = fi_eq_read(eq, &event, &entry, sizeof(entry), 2000);
  if (ret == -FI_EAGAIN) {
    return 0; 
  } else if (ret < 0) {
    fi_eq_err_entry err_entry;
    fi_eq_readerr(eq, &err_entry, event);
    return 0; 
  } else {
    entry.fid = &eq->fid;
    if (event == FI_CONNREQ) {
      event_map[&(eq->fid)]->handle_event(ACCEPT_EVENT, &entry); 
    } else if (event == FI_CONNECTED)  {
      event_map[&(eq->fid)]->handle_event(CONNECTED_EVENT, &entry);
    } else if (event == FI_SHUTDOWN) {
      event_map[&(eq->fid)]->handle_event(CLOSE_EVENT, &entry); 
    } else {
    }
  }
  return 0;
}

int EqDemultiplexer::register_event(fid* id) {
  int fd;
  if (fi_control(id, FI_GETWAIT, (void*)&fd)) {
    perror("fi_control");
    goto quit_add_event;
  }
  event.events = EPOLLIN;
  event.data.ptr = id;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0) {
    perror("epoll_ctl");
    goto quit_add_event;
  }
  return 0;
quit_add_event:
  close(fd);
  return -1;
}

int EqDemultiplexer::remove_event(fid* id) {
  int fd;
  if (fi_control(id, FI_GETWAIT, (void*)&fd)) {
    perror("fi_control");
    goto quit_delete_event;
  }
  event.events = EPOLLIN;
  event.data.ptr = id;
  if (epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event) < 0) {
    perror("epoll_ctl");
    goto quit_delete_event;
  }
  return 0;
quit_delete_event:
  close(fd);
  return -1;
}

void EqDemultiplexer::shutdown() {
}
