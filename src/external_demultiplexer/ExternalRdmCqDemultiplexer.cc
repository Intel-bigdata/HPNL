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

#include <cassert>

#include <chrono>
#include <iostream>

#include "core/RdmConnection.h"
#include "core/RdmStack.h"
#include "demultiplexer/EventType.h"
#include "external_demultiplexer/ExternalRdmCqDemultiplexer.h"

ExternalRdmCqDemultiplexer::ExternalRdmCqDemultiplexer(RdmStack* stack_, int index_)
    : stack(stack_), index(index_), start(0), end(0) {}

ExternalRdmCqDemultiplexer::~ExternalRdmCqDemultiplexer() {
#ifdef __linux__
  close(epfd);
#endif
}

int ExternalRdmCqDemultiplexer::init() {
  cqs = stack->get_cqs();
#ifdef __linux__
  fabric = stack->get_fabric();
  epfd = epoll_create1(0);
  memset((void*)&event, 0, sizeof event);
  int ret = fi_control(&cqs[index]->fid, FI_GETWAIT, (void*)&fd);
  if (ret) {
    std::cout << "fi_controll error." << std::endl;
    return -1;
  }
  event.events = EPOLLIN;
  event.data.ptr = &cqs[index]->fid;
  ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
  if (ret) {
    std::cout << "epoll add error." << std::endl;
    return -1;
  }
#endif
  return 0;
}

int ExternalRdmCqDemultiplexer::wait_event(Chunk** ck, int* block_buffer_size) {
  struct fid* fids[1];
  fids[0] = &cqs[index]->fid;
  int ret = 0;
#ifdef __linux__
  if (end - start >= 2000000) {
    if (fi_trywait(fabric, fids, 1) == FI_SUCCESS) {
      int epoll_ret = epoll_wait(epfd, &event, 1, 2000);
      if (event.data.ptr != (void*)&cqs[index]->fid) {
        std::cout << "Epoll wait error." << std::endl;
      }
      if (epoll_ret <= 0) {
        return 0;
      }
    }
    start = std::chrono::high_resolution_clock::now().time_since_epoch() /
            std::chrono::microseconds(1);
  }
#endif
  fi_cq_msg_entry entry;
  ret = fi_cq_read(cqs[index], &entry, 1);
  if (ret < 0 && ret != -FI_EAGAIN) {
    fi_cq_err_entry err_entry{};
    int err_res = fi_cq_readerr(cqs[index], &err_entry, entry.flags);
    if (err_res < 0) {
      perror("fi_cq_read");
    } else {
      const char* err_str =
          fi_cq_strerror(cqs[index], err_entry.prov_errno, err_entry.err_data, nullptr, 0);
      std::cerr << "fi_cq_read: " << err_str << std::endl;
    }
  } else if (ret > 0) {
    end = start;
    if (entry.flags & FI_RECV) {
      fi_context2* ctx = (fi_context2*)entry.op_context;
      *ck = (Chunk*)ctx->internal[4];
      *block_buffer_size = entry.len;
      return RECV_EVENT;
    } else if (entry.flags & FI_SEND) {
      fi_context2* ctx = (fi_context2*)entry.op_context;
      if (ctx->internal[4] == NULL) {
        std::free(ctx);
      } else {
        *ck = (Chunk*)ctx->internal[4];
      }
      return SEND_EVENT;
    } else if (entry.flags & FI_READ) {
      return READ_EVENT;
    } else if (entry.flags & FI_WRITE) {
      return WRITE_EVENT;
    } else {
      return 0;
    }
  }
  end = std::chrono::high_resolution_clock::now().time_since_epoch() /
        std::chrono::microseconds(1);
  return 0;
}
