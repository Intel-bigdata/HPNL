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

#include "external_demultiplexer/ExternalCqDemultiplexer.h"
#include "core/MsgStack.h"
#include "core/MsgConnection.h"
#include "demultiplexer/EventType.h"

#include <iostream>

ExternalCqDemultiplexer::ExternalCqDemultiplexer(MsgStack *stack_, fid_cq *cq_) : stack(stack_), cq(cq_), start(0), end(0) {}

ExternalCqDemultiplexer::~ExternalCqDemultiplexer() {
  #ifdef __linux__
  close(epfd);
  #endif
}

int ExternalCqDemultiplexer::init() {
  #ifdef __linux__
  fabric = stack->get_fabric();
  if ((epfd = epoll_create1(0)) == -1) {
    perror("epoll_create1");
    return -1;
  }
  memset((void*)&event, 0, sizeof event);
  if (fi_control(&cq->fid, FI_GETWAIT, (void*)&fd)) {
    perror("fi_control");
    return -1;
  }
  event.events = EPOLLIN;
  event.data.ptr = &cq->fid;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0) {
    perror("epoll_ctl");
    return -1;
  }
  #endif
  return 0;
}

int ExternalCqDemultiplexer::wait_event(fid_eq** eq, Chunk** ck, int* buffer_id, int* block_buffer_size) {
  struct fid *fids[1];
  fids[0] = &cq->fid;
  int ret = 0;
  #ifdef __linux__
  if (end - start >= 200) {
    if (fi_trywait(fabric, fids, 1) == FI_SUCCESS) {
      int epoll_ret = epoll_wait(epfd, &event, 1, 200);
      if (epoll_ret > 0) {
        assert(event.data.ptr == (void*)&cq->fid);
      } else if (epoll_ret < 0) {
        if (errno != EINTR) {
          perror("epoll_wait");
          return -1;
        }
        return 0;
      } else {
        return 0; 
      }
    }
    start = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  }
  #endif
  fi_cq_msg_entry entry;
  ret = fi_cq_read(cq, &entry, 1);
  if (ret < 0 && ret != -FI_EAGAIN) {
    fi_cq_err_entry err_entry{};
    int err_res = fi_cq_readerr(cq, &err_entry, entry.flags);
    if (err_res < 0) {
      perror("fi_cq_read");
    } else {
      const char *err_str = fi_cq_strerror(cq, err_entry.prov_errno, err_entry.err_data, nullptr, 0);
      std::cerr << "fi_cq_read: " << err_str << std::endl;
    }
  } else if (ret > 0) {
    end = start;
    *ck = (Chunk*)entry.op_context;
    *buffer_id = (*ck)->buffer_id;
    MsgConnection *con = (MsgConnection*)(*ck)->con;
    if (!con) {
      return 0;
    }
    fid_eq *eq_tmp = (fid_eq*)con->get_eq();
    *eq = eq_tmp;
    if (entry.flags & FI_RECV) {
      if (con->status < CONNECTED) {
        std::unique_lock<std::mutex> l(con->con_mtx);
        con->con_cv.wait(l, [con] { return con->status >= CONNECTED; });
        l.unlock();
      }
      con->recv((char*)(*ck)->buffer, entry.len);
      *block_buffer_size = entry.len;
      return RECV_EVENT;
    } else if (entry.flags & FI_SEND) {
      return SEND_EVENT;
    } else if (entry.flags & FI_READ) {
      return READ_EVENT;
    } else if (entry.flags & FI_WRITE) {
      return WRITE_EVENT;
    } else {
      return 0;
    }
  }
  end = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  return 0;
}
