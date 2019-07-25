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

#ifndef SERVICE
#define SERVICE

#include <cassert>

#include "HPNL/Callback.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Common.h"

class AcceptRequestCallback;
class Stack;
class MsgStack;
class Proactor;
class EqDemultiplexer;
class CqDemultiplexer;
class RdmCqDemultiplexer;
class EqHandler;
class EqThread;
class CqThread;
class RdmCqThread;
class RdmConnection;
class Connection;

class Service {
 public:
  Service(int /*worker_num_*/, int /*buffer_num_*/, bool is_server_ = false);
  ~Service();
  Service(const Service& service) = delete;
  Service& operator=(const Service& service) = delete;

  // Connection management
  int init(bool msg_ = true);
  int listen(const char* /*addr*/, const char* /*port*/);
  int connect(const char* /*addr*/, const char* /*port*/);
  void shutdown();
  void shutdown(Connection* con);

  // Service management
  void start();
  void wait();

  // Initialize buffer container
  void set_buf_mgr(ChunkMgr* /*bufMgr_*/);

  // Initialize event callback
  void set_send_callback(Callback* /*callback*/);
  void set_recv_callback(Callback* /*callback*/);
  void set_read_callback(Callback* /*callback*/);
  void set_connected_callback(Callback* /*callback*/);
  void set_shutdown_callback(Callback* /*callback*/);

  // RMA buffer registration
  uint64_t reg_rma_buffer(char* /*buffer*/, uint64_t /*buffer_size*/, int /*buffer_id*/);
  void unreg_rma_buffer(int /*buffer_id*/);
  Chunk* get_rma_buffer(int /*buffer_id*/);

  // Other util functions
  RdmConnection* get_con(const char* /*addr*/, const char* /*port*/);
  fid_domain* get_domain();

 private:
  friend class AcceptRequestCallback;

  Stack* stack;
  Proactor* proactor;
  EqDemultiplexer* eq_demultiplexer;
  CqDemultiplexer* cq_demultiplexer[MAX_WORKERS]{};
  RdmCqDemultiplexer* rdm_cq_demultiplexer[MAX_WORKERS]{};

  ChunkMgr* bufMgr{};

  Callback* recvCallback;
  Callback* sendCallback;
  Callback* readCallback;
  Callback* acceptRequestCallback;
  Callback* connectedCallback;
  Callback* shutdownCallback;

  int worker_num;
  int buffer_num;
  bool is_server;
  bool msg;

  EqThread* eqThread;
  CqThread* cqThread[MAX_WORKERS]{};
  RdmCqThread* rdmCqThread[MAX_WORKERS]{};
};

class AcceptRequestCallback : public Callback {
 public:
  explicit AcceptRequestCallback(Service* ioService_) : ioService(ioService_) {}
  ~AcceptRequestCallback() override = default;
  void operator()(void* param_1, void* param_2) override {
    assert(ioService->bufMgr);
    auto bufMgr = static_cast<ChunkMgr**>(param_1);
    *bufMgr = ioService->bufMgr;
  }

 private:
  Service* ioService;
};

#endif
