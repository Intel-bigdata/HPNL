// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include <iostream>

#include "core/MsgStack.h"
#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/EventHandler.h"

EqDemultiplexer::EqDemultiplexer(MsgStack* stack_) : stack(stack_) {}

EqDemultiplexer::~EqDemultiplexer() {
#ifdef __linux__
  close(epfd);
#endif
}

int EqDemultiplexer::init() {
#ifdef __linux__
  fabric = stack->get_fabric();
  epfd = epoll_create1(0);
  if (epfd == -1) {
    perror("epoll_create1");
    return -1;
  }
  memset((void*)&event, 0, sizeof event);
#endif
  return 0;
}

int EqDemultiplexer::wait_event(std::map<fid*, std::shared_ptr<EventHandler>> event_map) {
  if (event_map.empty()) return 0;
  uint32_t event_type;
  fi_eq_cm_entry entry{};
  fid_eq* eq = nullptr;
#ifdef __linux__
  struct fid* fids[event_map.size()];
  int i = 0;
  for (auto iter : event_map) {
    fids[i++] = iter.first;
  }
  if (fi_trywait(fabric, fids, i) == FI_SUCCESS) {
    int epoll_ret = epoll_wait(epfd, &event, 1, 200);
    if (epoll_ret > 0) {
      if (event_map[(fid *) event.data.ptr] != nullptr) {
        eq = event_map[(fid *) event.data.ptr]->get_handle();
      }
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
  if (!eq) {
    return 0;
  }

  int ret = fi_eq_read(eq, &event_type, &entry, sizeof(entry), 0);
  if (ret == -FI_EAGAIN) {
    return 0;
  } else if (ret < 0) {
    fi_eq_err_entry err_entry;
    fi_eq_readerr(eq, &err_entry, event_type);
    return 0;
  } else {
    entry.fid = &eq->fid;
    if (event_type == FI_CONNREQ) {
      event_map[&(eq->fid)]->handle_event(ACCEPT_EVENT, &entry);
    } else if (event_type == FI_CONNECTED) {
      event_map[&(eq->fid)]->handle_event(CONNECTED_EVENT, &entry);
    } else if (event_type == FI_SHUTDOWN) {
      event_map[&(eq->fid)]->handle_event(CLOSE_EVENT, &entry);
    } else {
    }
  }
#elif __APPLE__
  for (auto e : event_map) {
    eq = e.second->get_handle();
    int ret = fi_eq_read(eq, &event_type, &entry, sizeof(entry), 2000);
    if (ret == -FI_EAGAIN) {
    } else if (ret < 0) {
      fi_eq_err_entry err_entry{};
      fi_eq_readerr(eq, &err_entry, event_type);
    } else {
      entry.fid = &eq->fid;
      if (event_type == FI_CONNREQ) {
        event_map[&(eq->fid)]->handle_event(ACCEPT_EVENT, &entry);
      } else if (event_type == FI_CONNECTED) {
        event_map[&(eq->fid)]->handle_event(CONNECTED_EVENT, &entry);
      } else if (event_type == FI_SHUTDOWN) {
        event_map[&(eq->fid)]->handle_event(CLOSE_EVENT, &entry);
      } else {
      }
    }
  }
#endif
  return 0;
}

int EqDemultiplexer::register_event(fid* id) {
#ifdef __linux__
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
#endif
  return 0;
#ifdef __linux__
quit_add_event:
  close(fd);
  return -1;
#endif
}

int EqDemultiplexer::remove_event(fid* id) {
#ifdef __linux__
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
#endif
  return 0;
#ifdef __linux__
quit_delete_event:
  close(fd);
  return -1;
#endif
}
