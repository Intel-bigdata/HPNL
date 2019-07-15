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

#ifndef RDMCONNECTION_H
#define RDMCONNECTION_H

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <string.h>

#include <atomic>
#include <map>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "HPNL/ChunkMgr.h"
#include "core/ConnectionImpl.h"

class RdmConnection : public ConnectionImpl {
 public:
  RdmConnection(const char*, const char*, fi_info*, fid_domain*, fid_cq*, ChunkMgr*, int,
                bool, bool);
  ~RdmConnection() override;

  int init() override;
  int shutdown() override;

  int send(Chunk*) override;
  int send(int, int) override;
  int sendBuf(const char*, int) override;
  int sendTo(int, int, const char*) override;
  int sendBufTo(const char*, int, const char*) override;

  char* get_peer_name() override;
  char* get_local_name();
  int get_local_name_length();
  int activate_recv_chunk(Chunk* ck) override;
  std::vector<Chunk*> get_send_chunk();

  void set_recv_callback(Callback*) override;
  void set_send_callback(Callback*) override;
  Callback* get_recv_callback() override;
  Callback* get_send_callback() override;

  void log_used_chunk(Chunk* ck) override { used_chunks[ck->buffer_id] = (Chunk*)ck; }
  void remove_used_chunk(Chunk* ck) override { used_chunks.erase(ck->buffer_id); }

  void encode_(Chunk* ck, void* buffer, int buffer_length, char* peer_name) override;
  void decode_(Chunk* ck, void* buffer, int* buffer_length, char* peer_name) override;

 private:
  const char* ip;
  const char* port;

  fi_info* info;
  fid_domain* domain;
  fid_ep* ep;
  fid_av* av;
  fid_cq* conCq;

  int buffer_num;
  bool is_server;
  bool external_service;

  char local_name[64];
  size_t local_name_len = 64;
  std::map<std::string, fi_addr_t> address_map;

  std::vector<Chunk*> send_chunks;
  std::unordered_map<int, Chunk*> send_chunks_map;

  ChunkMgr* chunk_mgr;

  std::map<int, Chunk*> used_chunks;

  Callback* recv_callback;
  Callback* send_callback;
};

#endif
