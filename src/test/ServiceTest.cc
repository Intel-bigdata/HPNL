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

#include "HPNL/ChunkMgr.h"
#include "catch2/catch.hpp"
#include "service/Service.h"

TEST_CASE("msg server") {
  int total_buffer_num = 512;
  int single_buffer_num = 16;
  int buffer_size = 65536;
  auto bufMgr = new ExternalChunkMgr(total_buffer_num, buffer_size);
  auto service = new Service(1, single_buffer_num, true);
  SECTION("init") { REQUIRE(service->init(true) == 0); }
  SECTION("init->start->listen") {
    REQUIRE(service->init(true) == 0);
    service->set_chunk_mgr(bufMgr);
    service->start();
    REQUIRE(service->listen(nullptr, nullptr) == -1);
    service->shutdown();
  }
  SECTION("init->listen->start") {
    REQUIRE(service->init(true) == 0);
    service->set_chunk_mgr(bufMgr);
    REQUIRE(service->listen(nullptr, nullptr) == -1);
    service->start();
    service->shutdown();
  }
  delete service;
  delete bufMgr;
}

TEST_CASE("msg client") {
  int total_buffer_num = 512;
  int single_buffer_num = 16;
  int buffer_size = 65536;
  auto bufMgr = new ExternalChunkMgr(total_buffer_num, buffer_size);
  auto service = new Service(1, single_buffer_num, false);
  SECTION("init") { REQUIRE(service->init(true) == 0); }
  SECTION("init->start->connect") {
    REQUIRE(service->init(true) == 0);
    service->set_chunk_mgr(bufMgr);
    service->start();
    REQUIRE(service->connect(nullptr, nullptr) == -1);
    service->shutdown();
  }
  SECTION("init->connect->start") {
    REQUIRE(service->init(true) == 0);
    service->set_chunk_mgr(bufMgr);
    REQUIRE(service->connect(nullptr, nullptr) == -1);
    service->start();
    service->shutdown();
  }
  delete service;
  delete bufMgr;
}

TEST_CASE("rdm server") {
  int total_buffer_num = 512;
  int single_buffer_num = 16;
  int buffer_size = 65536;
  auto bufMgr = new ExternalChunkMgr(total_buffer_num, buffer_size);
  auto service = new Service(1, single_buffer_num, true);
  SECTION("init") { REQUIRE(service->init(false) == 0); }
  SECTION("init->start->listen") {
    REQUIRE(service->init(false) == 0);
    service->set_chunk_mgr(bufMgr);
    service->start();
    REQUIRE(service->listen(nullptr, nullptr) == -1);
    service->shutdown();
  }
  SECTION("init->listen->start") {
    REQUIRE(service->init(false) == 0);
    service->set_chunk_mgr(bufMgr);
    REQUIRE(service->listen(nullptr, nullptr) == -1);
    service->start();
    service->shutdown();
  }
  delete service;
  delete bufMgr;
}

TEST_CASE("rdm client") {
  int total_buffer_num = 512;
  int single_buffer_num = 16;
  int buffer_size = 65536;
  auto bufMgr = new ExternalChunkMgr(total_buffer_num, buffer_size);
  auto service = new Service(1, single_buffer_num, false);
  SECTION("init") { REQUIRE(service->init(false) == 0); }
  SECTION("init->start->connect") {
    REQUIRE(service->init(false) == 0);
    service->set_chunk_mgr(bufMgr);
    service->start();
    REQUIRE(service->get_con(nullptr, nullptr) == nullptr);
    service->shutdown();
  }
  delete service;
  delete bufMgr;
}
