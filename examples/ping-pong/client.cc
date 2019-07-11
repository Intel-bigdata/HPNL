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

#include <string.h>

#include "HPNL/Connection.h"
#include "HPNL/Client.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Callback.h"

#include <iostream>

#define MSG_SIZE 4096
#define BUFFER_SIZE (65536*2)
#define BUFFER_NUM 128

int count = 0;
uint64_t start, end = 0;
std::mutex mtx;

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

class ShutdownCallback : public Callback {
  public:
    explicit ShutdownCallback(Client *_clt) : clt(_clt) {}
    ~ShutdownCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      std::cout << "connection shutdown..." << std::endl;
      clt->shutdown();
    }
  private:
    Client *clt;
};

class ConnectedCallback : public Callback {
  public:
    explicit ConnectedCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~ConnectedCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      auto con = (Connection*)param_1;
      Chunk *ck = bufMgr->get();
      con->log_used_chunk(ck);
      ck->size = MSG_SIZE;
      memset(ck->buffer, '0', MSG_SIZE);
      con->send(ck);
    }
  private:
    ChunkMgr *bufMgr;
};

class RecvCallback : public Callback {
  public:
    RecvCallback(Client *client_, ChunkMgr *bufMgr_) : client(client_), bufMgr(bufMgr_) {}
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
      ck->size = MSG_SIZE;
      con->send(ck);

      Chunk *new_ck = bufMgr->get();
      con->log_used_chunk(new_ck);
      con->activate_recv_chunk(new_ck);
    }
  private:
    Client *client;
    ChunkMgr *bufMgr;
};

class SendCallback : public Callback {
  public:
    explicit SendCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~SendCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      bufMgr->reclaim(mid, ck);
      auto con = (Connection*)ck->con;
      con->remove_used_chunk(ck);
    }
  private:
    ChunkMgr *bufMgr;
};

int main(int argc, char *argv[]) {
  auto client = new Client(1, 16);
  client->init();

  ChunkMgr *bufMgr = new ChunkPool(client, BUFFER_SIZE, BUFFER_NUM, BUFFER_NUM*10);
  client->set_buf_mgr(bufMgr);

  auto recvCallback = new RecvCallback(client, bufMgr);
  auto sendCallback = new SendCallback(bufMgr);
  auto connectedCallback = new ConnectedCallback(bufMgr);
  auto shutdownCallback = new ShutdownCallback(client);

  client->set_recv_callback(recvCallback);
  client->set_send_callback(sendCallback);
  client->set_connected_callback(connectedCallback);
  client->set_shutdown_callback(shutdownCallback);

  client->start();
  client->connect("172.168.2.106", "12345");

  client->wait();

  delete shutdownCallback;
  delete connectedCallback;
  delete sendCallback;
  delete recvCallback;
  delete client;
  delete bufMgr;
  return 0;
}
