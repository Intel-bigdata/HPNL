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

#ifndef MSGCONNECTION_H
#define MSGCONNECTION_H

#include <string.h>

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_rma.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "HPNL/Callback.h"
#include "HPNL/ChunkMgr.h"
#include "core/ConnectionImpl.h"

enum ConStatus { IDLE = 0, CONNECT_REQ, ACCEPT_REQ, CONNECTED, SHUTDOWN_REQ, DOWN };

class MsgStack;

class MsgConnection : public ConnectionImpl {
 public:
  MsgConnection(MsgStack*, fid_fabric*, fi_info*, fid_domain*, fid_cq*, ChunkMgr*, bool,
                int, int, bool);
  ~MsgConnection() override;

  int init() override;
  int send(Chunk*) override;
  int read(Chunk*, int, uint64_t, uint64_t, uint64_t) override;
  int write(Chunk*, int, uint64_t, uint64_t, uint64_t) override;

  /// for java binding
  int send(int, int) override;

  int shutdown() override;
  int connect();
  int accept();

  int activate_recv_chunk(Chunk* = nullptr) override;

  void init_addr();
  void get_addr(char**, size_t*, char**, size_t*);
  int get_cq_index();
  bool external_ervice;
  fid_eq* get_eq();

  fid* get_fid();

  void set_recv_callback(Callback*) override;
  void set_send_callback(Callback*) override;
  void set_read_callback(Callback*);
  void set_write_callback(Callback*);
  void set_shutdown_callback(Callback*);

  Callback* get_recv_callback() override;
  Callback* get_send_callback() override;
  Callback* get_read_callback();
  Callback* get_write_callback();
  Callback* get_shutdown_callback();

  std::vector<Chunk*> get_send_chunks();
 public:
  ConStatus status;
  std::mutex con_mtx;
  std::condition_variable con_cv;

 private:
  MsgStack* stack;
  fid_fabric* fabric;
  fi_info* info;
  fid_domain* domain;
  fid_ep* ep;
  fid_cq* conCq;
  fid_eq* conEq;

  bool is_server;

  int buffer_num;
  int cq_index;

  ChunkMgr* chunk_mgr;

  size_t dest_port;
  char dest_addr[20];
  size_t src_port;
  char src_addr[20];

  Callback* recv_callback;
  Callback* send_callback;
  Callback* read_callback;
  Callback* write_callback;
  Callback* shutdown_callback;
  // for Java interface
  std::vector<Chunk*> send_chunks;
};

#endif
