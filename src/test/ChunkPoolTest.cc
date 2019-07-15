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

TEST_CASE("chunk pool") {
  int request_chunk_number = 32;
  auto cp = new ChunkPool(nullptr, 4096, request_chunk_number * 2, 1024);
  for (int i = 0; i < request_chunk_number; i++) {
    auto ck = reinterpret_cast<Chunk*>(cp->get(nullptr));
    REQUIRE(ck != nullptr);
    REQUIRE(ck->buffer_id == i);
    REQUIRE(cp->get(ck->buffer_id) == ck);
  }
  delete cp;
}
