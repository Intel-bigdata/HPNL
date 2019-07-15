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

#ifndef CQEXTERNALMULTIPLEXER_H
#define CQEXTERNALMULTIPLEXER_H

#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <rdma/fi_cm.h>
#include <unistd.h>

#include "HPNL/ChunkMgr.h"

class MsgStack;

class ExternalCqDemultiplexer {
 public:
  ExternalCqDemultiplexer(MsgStack*, fid_cq*);
  ~ExternalCqDemultiplexer();
  int init();
  int wait_event(fid_eq**, Chunk**, int*, int*);

 private:
  MsgStack* stack;
  fid_cq* cq;
  uint64_t start;
  uint64_t end;
#ifdef __linux__
  fid_fabric* fabric;
  struct epoll_event event;
  int epfd;
  int fd;
#endif
};

#endif
