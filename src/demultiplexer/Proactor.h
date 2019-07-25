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

#ifndef REACTOR_H
#define REACTOR_H

#include <list>
#include <map>

#include "HPNL/Common.h"
#include "demultiplexer/ThreadWrapper.h"

class EqDemultiplexer;
class CqDemultiplexer;
class RdmCqDemultiplexer;
class EventHandler;
class fid;

class Proactor {
 public:
  Proactor(EqDemultiplexer*, CqDemultiplexer**, int);
  Proactor(RdmCqDemultiplexer**, int);
  ~Proactor();
  int eq_service();
  int cq_service(int);
  int rdm_cq_service(int);
  int register_handler(std::shared_ptr<EventHandler>);
  int remove_handler(std::shared_ptr<EventHandler>);
  int remove_handler(fid*);
  int handle_events(int timeout = 0);

 private:
  std::mutex mtx;
  std::map<fid*, std::shared_ptr<EventHandler>> eventMap;
  std::map<fid*, std::shared_ptr<EventHandler>> curEventMap;
  EqDemultiplexer* eqDemultiplexer;
  CqDemultiplexer* cqDemultiplexer[MAX_WORKERS];
  RdmCqDemultiplexer* rdmCqDemultiplexer[MAX_WORKERS];
  int cq_worker_num;
};

class EqThread : public ThreadWrapper {
 public:
  EqThread(Proactor* proactor_) : proactor(proactor_) {}
  virtual ~EqThread() {}
  virtual int entry() override { return proactor->eq_service(); }
  virtual void abort() override {}

 private:
  Proactor* proactor;
};

class CqThread : public ThreadWrapper {
 public:
  CqThread(Proactor* proactor_, int index_) : proactor(proactor_), index(index_) {}
  virtual ~CqThread() {}
  virtual int entry() override { return proactor->cq_service(index); }
  virtual void abort() override {}

 private:
  Proactor* proactor;
  int index;
};

class RdmCqThread : public ThreadWrapper {
 public:
  RdmCqThread(Proactor* proactor_, int index_) : proactor(proactor_), index(index_) {}
  virtual ~RdmCqThread() {}
  virtual int entry() override { return proactor->rdm_cq_service(index); }
  virtual void abort() override {}

 private:
  Proactor* proactor;
  int index;
};

#endif
