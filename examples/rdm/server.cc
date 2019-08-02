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

#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "HPNL/Server.h"

#include <chrono>
#include <thread>

#include <iostream>

#define BUFFER_SIZE 65536
#define BUFFER_NUM 1024
#define MSG_SIZE 4096

class RecvCallback : public Callback {
  public:
    explicit RecvCallback(ChunkMgr *chunkMgr_) : chunkMgr(chunkMgr_) {}
    ~RecvCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = chunkMgr->get(mid);
      auto con = (Connection*)ck->con;
      char peer_name[16];
      con->decode_(ck, nullptr, nullptr, peer_name);
      con->encode_(ck, ck->buffer, MSG_SIZE, peer_name);
      con->send(ck);
    }
  private:
    ChunkMgr *chunkMgr;
};

class SendCallback : public Callback {
  public:
    explicit SendCallback(ChunkMgr *chunkMgr_) : chunkMgr(chunkMgr_) {}
    ~SendCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = chunkMgr->get(mid);
      auto con = (Connection*)ck->con;
      chunkMgr->reclaim(ck, con);
    }
  private:
    ChunkMgr *chunkMgr;
};

int main() {

  auto server = new Server(1, 16);
  server->init(false);

  ChunkMgr *chunkMgr = new ChunkPool(server, BUFFER_SIZE, BUFFER_NUM);
  server->set_chunk_mgr(chunkMgr);

  auto recvCallback = new RecvCallback(chunkMgr);
  auto sendCallback = new SendCallback(chunkMgr);
  server->set_recv_callback(recvCallback);
  server->set_send_callback(sendCallback);

  server->listen("127.0.0.1", "12345");
  server->start();

  server->wait();

  delete recvCallback;
  delete sendCallback;
  delete server;
  delete chunkMgr;
  return 0;
}
