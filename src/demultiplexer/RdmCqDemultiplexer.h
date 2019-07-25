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

#ifndef RDMCQDEMLUTIPLEXER_H
#define RDMCQDEMLUTIPLEXER_H

#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <unistd.h>

#include <HPNL/ChunkMgr.h>

class RdmStack;

class RdmCqDemultiplexer {
 public:
  RdmCqDemultiplexer(RdmStack*, int);
  ~RdmCqDemultiplexer();
  int init();
  int wait_event();

 private:
  RdmStack* stack;
  int index;
  fid_cq** cqs;
#ifdef __linux__
  fid_fabric* fabric;
  struct epoll_event event;
  int epfd;
  int fd;
#endif
};

#endif
