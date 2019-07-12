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
#include "HPNL/Client.h"

#include <iostream>

int count = 0;
uint64_t start, end = 0;
std::mutex mtx;

#define BUFFER_SIZE 65536
#define BUFFER_NUM 1024
#define MSG_SIZE 4096

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

class RecvCallback : public Callback {
  public:
    explicit RecvCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~RecvCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      std::lock_guard<std::mutex> lk(mtx);
      count++;
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      auto con = (Connection*)ck->con;
      if (count >= 1000000) {
        end = timestamp_now();
        printf("finished, totally consumes %f s, message round trip time is %f us.\n", (end-start)/1000.0, (end-start)*1000/1000000.0);
        return;
      }
      if (count == 1) {
        printf("start ping-pong.\n");
      }
      if (count == 1) {
        start = timestamp_now(); 
      }
      char peer_name[16];
      con->decode_(ck, nullptr, nullptr, peer_name);
      con->encode_(ck, ck->buffer, MSG_SIZE, peer_name);
      con->send(ck);
    }
  private:
    ChunkMgr *bufMgr;
};

class SendCallback : public Callback {
  public:
    explicit SendCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~SendCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      auto con = (Connection*)ck->con;
      bufMgr->reclaim(ck, con);
    }
  private:
    ChunkMgr *bufMgr;
};

int main() {
  auto client = new Client(1, 16);
  client->init(false);

  ChunkMgr *bufMgr = new ChunkPool(client, BUFFER_SIZE, BUFFER_NUM, BUFFER_NUM*10);
  client->set_buf_mgr(bufMgr);

  auto recvCallback = new RecvCallback(bufMgr);
  auto sendCallback = new SendCallback(bufMgr);
  client->set_recv_callback(recvCallback);
  client->set_send_callback(sendCallback);

  client->start();

  Connection *con = client->get_con("127.0.0.1", "12345");
  assert(con);
  char* buffer = (char*)std::malloc(MSG_SIZE);
  memset(buffer, '0', MSG_SIZE);
  
  char* peer_name = con->get_peer_name();
  auto ck = bufMgr->get(con);
  con->encode_(ck, buffer, MSG_SIZE, peer_name);
  con->send(ck);

  client->wait();

  delete recvCallback;
  delete sendCallback;
  delete client;
  delete bufMgr;

  return 0;
}
