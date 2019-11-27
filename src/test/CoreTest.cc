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

#define CATCH_CONFIG_MAIN

#include <vector>

#include "HPNL/ChunkMgr.h"
#include "HPNL/Common.h"
#include "catch2/catch.hpp"
#include "core/MsgConnection.h"
#include "core/MsgStack.h"
#include "core/RdmStack.h"

TEST_CASE("msg server") {
  auto stack = new MsgStack(1, 6, true, true);
  SECTION("init") { REQUIRE(stack->init() == 0); }
  SECTION("init->bind->listen") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->bind(nullptr, nullptr, nullptr) == nullptr);
    REQUIRE(stack->listen() == -1);
  }
  SECTION("init->listen") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->listen() == -1);
  }
  SECTION("bind->listen") {
    REQUIRE(stack->bind(nullptr, nullptr, nullptr) == nullptr);
    REQUIRE(stack->listen() == -1);
  }
  SECTION("listen") { REQUIRE(stack->listen() == -1); }

  delete stack;
}

TEST_CASE("msg client") {
  int buffer_num_per_connection = 16;
  auto stack = new MsgStack(1, buffer_num_per_connection, false, true);
  SECTION("init") { REQUIRE(stack->init() == 0); }
  SECTION("init->connect") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->connect(nullptr, nullptr, nullptr) == nullptr);
  }
  SECTION("connect") { REQUIRE(stack->connect(nullptr, nullptr, nullptr) == nullptr); }
  delete stack;
}

#if defined(LOCAL_IP) && defined(LOCAL_PORT)
void connect(MsgStack* stack, TestBufMgr* mgr, int buffer_num_per_connection) {
  fid_eq* eq = stack->connect(LOCAL_IP, LOCAL_PORT, mgr);
  MsgConnection* con = stack->get_connection(&eq->fid);
  REQUIRE(con != nullptr);
  REQUIRE(con->get_send_buffer().size() == buffer_num_per_connection);
}
#endif

TEST_CASE("msg connect operation") {
  int parallel_num = 10;
  int buffer_num_per_connection = 16;
  int total_buffer_num = parallel_num * buffer_num_per_connection * 2;
  int buffer_size = 65536;

  auto mgr = new ExternalChunkMgr(total_buffer_num, buffer_size);
  auto stack = new MsgStack(1, buffer_num_per_connection, false, true);
  REQUIRE(stack->init() == 0);

#if defined(LOCAL_IP) && defined(LOCAL_PORT)
  SECTION("multiple connection") {
    std::vector<std::thread> threads;
    for (int i = 0; i < parallel_num; i++) {
      threads.push_back(std::thread(connect, stack, mgr, buffer_num_per_connection));
      // Don't support parallel connection
      threads[i].join();
    }
  }
#endif
  delete stack;
  delete mgr;
}

void reg_rma_buffer(MsgStack* stack, char* test, int i) {
  stack->reg_rma_buffer(test, i * 10, i);
}

void unreg_rma_buffer(MsgStack* stack, int i) { stack->unreg_rma_buffer(i); }

TEST_CASE("rma buffer registration") {
  char test[10] = "12345";
  int parallel_num = 100;
  auto stack = new MsgStack(1, 6, false, true);
  stack->init();

  SECTION("async rma buffer registration") {
    std::vector<std::thread> threads;
    threads.reserve(parallel_num);
    for (int i = 0; i < parallel_num; i++) {
      threads.emplace_back(reg_rma_buffer, stack, test, i);
    }

    for (int i = 0; i < parallel_num; i++) {
      threads[i].join();
    }

    REQUIRE(stack->get_rma_chunk(5)->capacity == 50);
    REQUIRE(stack->get_rma_chunk(2)->capacity == 20);

    for (int i = 0; i < parallel_num; i++) {
      stack->unreg_rma_buffer(i);
    }

    REQUIRE(stack->get_rma_chunk(5) == nullptr);
    REQUIRE(stack->get_rma_chunk(2) == nullptr);
  }

  SECTION("sycn rma buffer registration") {
    REQUIRE(stack->reg_rma_buffer(nullptr, 1, 2) == nullptr);
    REQUIRE(stack->reg_rma_buffer(test, 10, 1)->mr->key > 0);
    REQUIRE(stack->reg_rma_buffer(test, 20, 2)->mr->key > 0);
    REQUIRE(stack->reg_rma_buffer(test, 30, 3)->mr->key > 0);
    REQUIRE(stack->reg_rma_buffer(test, 40, 4)->mr->key > 0);
    REQUIRE(stack->get_rma_chunk(1)->capacity == 10);
    REQUIRE(stack->get_rma_chunk(2)->capacity == 20);
    REQUIRE(stack->get_rma_chunk(3)->capacity == 30);
    REQUIRE(stack->get_rma_chunk(4)->capacity == 40);
    stack->unreg_rma_buffer(1);
    stack->unreg_rma_buffer(2);
    stack->unreg_rma_buffer(3);
    stack->unreg_rma_buffer(4);
    REQUIRE(stack->get_rma_chunk(1) == nullptr);
    REQUIRE(stack->get_rma_chunk(2) == nullptr);
    REQUIRE(stack->get_rma_chunk(3) == nullptr);
    REQUIRE(stack->get_rma_chunk(4) == nullptr);
  }

  delete stack;
}

TEST_CASE("rdm server") {
  auto stack = new RdmStack(16, true, false);
  SECTION("init->init") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->bind(nullptr, nullptr, nullptr) == nullptr);
  }
  SECTION("bind") { REQUIRE(stack->bind(nullptr, nullptr, nullptr) == nullptr); }
  delete stack;
}

TEST_CASE("rdm client") {
  auto stack = new RdmStack(16, false, false);
  SECTION("init->get_con") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->get_con(nullptr, nullptr, nullptr) == nullptr);
  }
  SECTION("get_con") { REQUIRE(stack->get_con(nullptr, nullptr, nullptr) == nullptr); }
  delete stack;
}
