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

#include <stdlib.h>
#include <string.h>

#include "HPNL/Connection.h"
#include "HPNL/Client.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Callback.h"

#include <iostream>

#define SIZE 4096
#define BUFFER_SIZE 65536
#define BUFFER_NUM 65536

int count = 0;
long addr, rkey, len;
char rma_buf[4096];

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback(Client *_clt) : clt(_clt) {}
    virtual ~ShutdownCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      std::cout << "connection shutdown..." << std::endl;
      clt->shutdown();
    }
  private:
    Client *clt;
};

class ConnectedCallback : public Callback {
  public:
    ConnectedCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~ConnectedCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      Connection *con = (Connection*)param_1;
      char* buffer = (char*)std::malloc(SIZE);
      memset(buffer, '0', SIZE);
      con->sendBuf(buffer, SIZE);
      std::free(buffer);
    }
  private:
    ChunkMgr *bufMgr;
};

class RecvCallback : public Callback {
  public:
    RecvCallback(ChunkMgr *bufMgr_, Client *client_) : bufMgr(bufMgr_), client(client_) {}
    virtual ~RecvCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);

      char* buffer = (char*)std::malloc(SIZE);
      memset(buffer, '0', SIZE);
      Connection *con = (Connection*)ck->con;

      if (count == 0) {
        char* buf = (char*)ck->buffer;
        addr = atol(buf);
        con->sendBuf(buffer, SIZE);
      } else if (count == 1){
        char* buf = (char*)ck->buffer;
        rkey = atol(buf);
        con->sendBuf(buffer, SIZE);
      } else {
        char* buf = (char*)ck->buffer;
        len = atol(buf);
        client->reg_rma_buffer(rma_buf, 4096, 0);
        con->read(0, 0, len, addr, rkey);
      }
      count++;
    }
  private:
    ChunkMgr *bufMgr;
    Client *client;
};

class SendCallback : public Callback {
  public:
    SendCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~SendCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      Connection *con = (Connection*)ck->con;
      con->activate_send_chunk(ck);
    }
  private:
    ChunkMgr *bufMgr;
};

class ReadCallback : public Callback {
  public:
    ReadCallback() {}
    virtual ~ReadCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      std::cout << "rma buffer " << rma_buf << std::endl;
    }
};

int main(int argc, char *argv[]) {
  Client *client = new Client(1, 16);
  ChunkMgr *bufMgr = new ChunkPool(client, BUFFER_SIZE, BUFFER_NUM, BUFFER_NUM*10);

  client->init();
  client->set_buf_mgr(bufMgr);

  RecvCallback *recvCallback = new RecvCallback(bufMgr, client);
  SendCallback *sendCallback = new SendCallback(bufMgr);
  ReadCallback *readCallback = new ReadCallback();
  ConnectedCallback *connectedCallback = new ConnectedCallback(bufMgr);
  ShutdownCallback *shutdownCallback = new ShutdownCallback(client);

  client->set_recv_callback(recvCallback);
  client->set_send_callback(sendCallback);
  client->set_read_callback(readCallback);
  client->set_connected_callback(connectedCallback);
  client->set_shutdown_callback(shutdownCallback);

  client->start();
  client->connect("172.168.0.40", "123456");

  client->wait();

  delete shutdownCallback;
  delete connectedCallback;
  delete sendCallback;
  delete recvCallback;
  delete client;
  delete bufMgr;

  return 0;
}
