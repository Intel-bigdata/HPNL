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

#include <cstring>
#include <iostream>
#include <libpmemobj.h>
#include <fcntl.h>

#include "format_generated.h"

/* size of the pmemobj pool -- 1 GB */
#define POOL_SIZE 126833655808L

/* name of our layout in the pool */
#define LAYOUT_NAME "pmem_spark_shuffle"

#define MSG_SIZE 4096
#define BUFFER_SIZE (65536 * 2)
#define BUFFER_NUM 128

struct my_root {
  size_t len;
  char buf[];
};

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
  explicit RecvCallback(Server* server_, ChunkMgr* chunkMgr_)
      : server(server_), chunkMgr(chunkMgr_) {}
  ~RecvCallback() override = default;
  void operator()(void* param_1, void* param_2) override {
    const char path[] = "/dev/dax0.0";

    int sds_write_value = 0;
    pmemobj_ctl_set(nullptr, "sds.at_create", &sds_write_value);

    /* create the pmemobj pool or open it if it already exists */
    pop = pmemobj_open(path, LAYOUT_NAME);
    if (pop == nullptr) {
      std::cout << "failed to open." << std::endl;
      pop = pmemobj_create(path, LAYOUT_NAME, 0, S_IRUSR | S_IWUSR);
    }

    if (pop == nullptr) {
      std::cout << "failed to create" << std::endl;
      exit(1);
    }

    PMEMoid root = pmemobj_root(pop, sizeof(struct my_root));
    rootp = static_cast<struct my_root*>(pmemobj_direct(root));

    char buf[] = "hello world";
    rootp->len = strlen(buf);

    pmemobj_memcpy_persist(pop, rootp->buf, buf, rootp->len);
    pmemobj_persist(pop, &rootp->len, sizeof (rootp->len));

    rma_ck = server->reg_rma_buffer(reinterpret_cast<char*>(pop), POOL_SIZE, 0);

    int mid = *static_cast<int*>(param_1);
    auto ck = chunkMgr->get(mid);
    ck->size = MSG_SIZE;
    auto con = static_cast<Connection*>(ck->con);

    flatbuffers::FlatBufferBuilder builder;
    auto msg =
        Createrma_msg(builder, (uint64_t)rootp->buf, rootp->len, 0, rma_ck->mr->key);
    builder.Finish(msg);
    uint8_t* pmof_buf = builder.GetBufferPointer();

    memcpy(ck->buffer, pmof_buf, builder.GetSize());
    ck->size = builder.GetSize();
    con->send(ck);
  }

 private:
  Server* server;
  ChunkMgr* chunkMgr;
  PMEMobjpool* pop;
  struct my_root* rootp;
  Chunk* rma_ck;
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

  auto recvCallback = new RecvCallback(server, chunkMgr);
  auto sendCallback = new SendCallback(chunkMgr);
  auto shutdownCallback = new ShutdownCallback();
  server->set_recv_callback(recvCallback);
  server->set_send_callback(sendCallback);
  server->set_connected_callback(nullptr);
  server->set_shutdown_callback(shutdownCallback);

  server->start();
  server->listen("172.168.0.40", "12345");

  server->wait();

  delete recvCallback;
  delete sendCallback;
  delete shutdownCallback;
  delete server;
  delete chunkMgr;
  return 0;
}
