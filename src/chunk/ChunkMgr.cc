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
#include <rdma/fi_domain.h>

char* PoolAllocator::malloc(const size_type bytes) {
  int chunk_size = buffer_size+sizeof(Chunk);
  int buffer_num = bytes/chunk_size;
  auto memory = static_cast<char*>(std::malloc(bytes));
  if (PoolAllocator::domain) {
    if (fi_mr_reg(domain, memory, bytes, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr, nullptr)) {
      perror("fi_mr_reg");
      return nullptr; 
    }
  }
  auto first = reinterpret_cast<Chunk*>(memory);
  auto memory_ptr = first;
  for (int i = 0; i < buffer_num; i++) {
    chunk_to_id_map[memory_ptr] = id;
    id_to_chunk_map[id] = memory_ptr;
    id++;
    memory_ptr = reinterpret_cast<Chunk*>(reinterpret_cast<char*>(memory_ptr)+chunk_size);
  }
  return reinterpret_cast<char *>(first);
}

void PoolAllocator::free(char* const block) {
  auto memory = reinterpret_cast<Chunk*>(block);
  std::free(memory);
}

int PoolAllocator::buffer_size = 0;
fid_domain* PoolAllocator::domain = nullptr;
fid_mr* PoolAllocator::mr = nullptr;
int PoolAllocator::id = 0;
std::map<int, Chunk*> PoolAllocator::id_to_chunk_map;
std::map<Chunk*, int> PoolAllocator::chunk_to_id_map;
std::mutex PoolAllocator::mtx;

ChunkPool::ChunkPool(FabricService* service, const int request_buffer_size,
  const int next_request_buffer_number, const int max_buffer_number) :
    pool(request_buffer_size+sizeof(Chunk), next_request_buffer_number, max_buffer_number),
    buffer_size(request_buffer_size), used_buffers(0) {

  PoolAllocator::buffer_size = buffer_size;
  if (service) {
    PoolAllocator::domain = service->get_domain();
  }
  PoolAllocator::id = 0;
}

ChunkPool::~ChunkPool() {
  for (auto ck : PoolAllocator::id_to_chunk_map) {
    if (ck.second->mr) {
      fi_close(&((fid_mr*)ck.second->mr)->fid);
    }
  }
  PoolAllocator::id_to_chunk_map.clear();
  PoolAllocator::chunk_to_id_map.clear();
}

void* ChunkPool::malloc() {
  if (!store().empty()) {
    return (store().malloc());
  }
  return system_malloc();
}

void ChunkPool::free(void * const ck) {
  (store().free)(ck);
}

Chunk* ChunkPool::get(int id) {
  std::lock_guard<std::mutex> l(PoolAllocator::mtx);
  if (!PoolAllocator::id_to_chunk_map.count(id)) {
    return nullptr;
  }
  return PoolAllocator::id_to_chunk_map[id];
}

Chunk* ChunkPool::get() {
  std::lock_guard<std::mutex> l(PoolAllocator::mtx);
  auto ck = reinterpret_cast<Chunk*>(pool::malloc());
  used_buffers++;
  ck->mr = PoolAllocator::mr;
  ck->capacity = buffer_size;
  ck->buffer_id = PoolAllocator::chunk_to_id_map[ck];
  ck->buffer = ck->data;
  ck->size = 0;
  return ck;
}

void ChunkPool::reclaim(int, Chunk* ck) {
  std::lock_guard<std::mutex> l(PoolAllocator::mtx);
  pool::free(ck);
  used_buffers--; 
}

int ChunkPool::free_size() {
  return INT_MAX;
}

void* ChunkPool::system_malloc() {
  return boost::pool<PoolAllocator>::malloc();
}
