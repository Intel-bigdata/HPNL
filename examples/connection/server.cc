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

#include "HPNL/Connection.h"
#include "HPNL/Server.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Callback.h"

#include <iostream>

#define MSG_SIZE 3
#define BUFFER_SIZE 65536
#define BUFFER_NUM 128
#define MAX_WORKERS 10

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback() = default;
    ~ShutdownCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      std::cout << "connection shutdown..." << std::endl;
    }
};

int main(int argc, char *argv[]) {
  auto server = new Server(1, 16);
  server->init();

  ChunkMgr *bufMgr = new ChunkPool(server, BUFFER_SIZE, BUFFER_NUM, BUFFER_NUM*10);
  server->set_buf_mgr(bufMgr);

  auto shutdownCallback = new ShutdownCallback();

  server->set_shutdown_callback(shutdownCallback);

  server->start();
  server->listen("172.168.2.106", "12345");
  server->wait();

  delete server;
  delete bufMgr;
  return 0;
}
