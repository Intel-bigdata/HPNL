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

#include "HPNL/Callback.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Connection.h"
#include "HPNL/Server.h"

#include <iostream>

#define MSG_SIZE 4096
#define BUFFER_SIZE (65536 * 2)
#define BUFFER_NUM 128

class ShutdownCallback : public Callback {
 public:
  ShutdownCallback() = default;
  ~ShutdownCallback() override = default;
  void operator()(void* param_1, void* param_2) override {
    std::cout << "connection shutdown..." << std::endl;
  }
};

class RecvCallback : public Callback {
 public:
  explicit RecvCallback(ChunkMgr* chunkMgr_) : chunkMgr(chunkMgr_) {}
  ~RecvCallback() override = default;
  void operator()(void* param_1, void* param_2) override {
    int mid = *static_cast<int*>(param_1);
    auto ck = chunkMgr->get(mid);
    auto con = static_cast<Connection*>(ck->con);
    ck->size = MSG_SIZE;
    con->send(ck);
  }

 private:
  ChunkMgr* chunkMgr;
};

class SendCallback : public Callback {
 public:
  explicit SendCallback(ChunkMgr* chunkMgr_) : chunkMgr(chunkMgr_) {}
  ~SendCallback() override = default;
  void operator()(void* param_1, void* param_2) override {
    int mid = *static_cast<int*>(param_1);
    Chunk* ck = chunkMgr->get(mid);
    auto con = static_cast<Connection*>(ck->con);
    chunkMgr->reclaim(ck, con);
  }

 private:
  ChunkMgr* chunkMgr;
};

int main(int argc, char* argv[]) {
  auto server = new Server(1, 16);
  server->init();

  ChunkMgr* chunkMgr = new ChunkPool(server, BUFFER_SIZE, BUFFER_NUM);
  server->set_chunk_mgr(chunkMgr);

  auto recvCallback = new RecvCallback(chunkMgr);
  auto sendCallback = new SendCallback(chunkMgr);
  auto shutdownCallback = new ShutdownCallback();
  server->set_recv_callback(recvCallback);
  server->set_send_callback(sendCallback);
  server->set_connected_callback(nullptr);
  server->set_shutdown_callback(shutdownCallback);

  server->start();
  server->listen("172.168.2.106", "12345");
  server->wait();

  delete recvCallback;
  delete sendCallback;
  delete shutdownCallback;
  delete server;
  delete chunkMgr;
  return 0;
}
