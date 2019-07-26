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

#include "HPNL/ChunkMgr.h"
#include "HPNL/Connection.h"

ExternalChunkMgr::ExternalChunkMgr() : buffer_num(0), buffer_size(0), buffer_id(0) {}

ExternalChunkMgr::ExternalChunkMgr(int buffer_num_, uint64_t buffer_size_) : buffer_num(buffer_num_), buffer_size(buffer_size_), buffer_id(0) {
  for (int i = 0; i < buffer_num*2; i++) {
    auto ck = new Chunk();
    ck->buffer = std::malloc(buffer_size);
    ck->capacity = buffer_size;
    ck->buffer_id = this->get_id();
    ck->mr = nullptr;
    this->reclaim(ck, nullptr);
  }
}

ExternalChunkMgr::~ExternalChunkMgr() {
  for (auto buf : buf_map) {
    std::free(buf.second->buffer);
    delete buf.second;
    buf.second = nullptr;
  }
  buf_map.clear();
}

Chunk* ExternalChunkMgr::get(int id) {
  std::lock_guard<std::mutex> l(mtx);
  return buf_map[id];
}

Chunk* ExternalChunkMgr::get(Connection* con) {
  std::lock_guard<std::mutex> l(mtx);
  if (bufs.empty())
    return nullptr;
  Chunk *ck = bufs.back();
  bufs.pop_back();
  if (!con)
    con->log_used_chunk(ck);
  return ck;
}

void ExternalChunkMgr::reclaim(Chunk* ck, Connection* con) {
  std::lock_guard<std::mutex> l(mtx);
  if (!buf_map.count(ck->buffer_id))
    buf_map[ck->buffer_id] = ck;
  bufs.push_back(ck);
  if (con) {
    con->remove_used_chunk(ck);
  }
}

int ExternalChunkMgr::free_size() {
  std::lock_guard<std::mutex> l(mtx);
  return bufs.size();
}

uint32_t ExternalChunkMgr::get_id() {
  return buffer_id++;
}
