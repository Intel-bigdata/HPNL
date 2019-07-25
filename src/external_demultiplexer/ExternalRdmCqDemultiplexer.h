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

#ifndef EXTERNALRDMCQDEMLUTIPLEXER_H
#define EXTERNALRDMCQDEMLUTIPLEXER_H

#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <unistd.h>

#include <HPNL/ChunkMgr.h>

class RdmStack;

class ExternalRdmCqDemultiplexer {
 public:
  ExternalRdmCqDemultiplexer(RdmStack*, int);
  ~ExternalRdmCqDemultiplexer();
  int init();
  int wait_event(Chunk**, int*);

 private:
  RdmStack* stack;
  int index;
  fid_cq** cqs;
  uint64_t start;
  uint64_t end;
#ifdef __linux__
  int epfd;
  int fd;
  struct epoll_event event;
  fid_fabric* fabric;
#endif
};

#endif
