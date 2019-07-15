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

#ifndef EQEVENTDEMULTIPLEXER_H
#define EQEVENTDEMULTIPLEXER_H

#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <unistd.h>

#include <string.h>
#include <atomic>
#include <map>
#include <memory>
#include <unordered_map>

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>

class EventHandler;
class MsgStack;

class EqDemultiplexer {
 public:
  explicit EqDemultiplexer(MsgStack*);
  ~EqDemultiplexer();
  int init();
  int wait_event(std::map<fid*, std::shared_ptr<EventHandler>> eventMap);
  int register_event(fid*);
  int remove_event(fid*);

 private:
  MsgStack* stack;
#ifdef __linux__
  fid_fabric* fabric;
  int epfd;
  struct epoll_event event;
#endif
};

#endif
