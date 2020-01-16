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
#include "HPNL/Connection.h"

char* PoolAllocator::malloc(const size_type bytes) {
  int chunk_size = chunkPoolContext->buffer_size + sizeof(Chunk);
  int buffer_num = bytes / chunk_size;
  char* memory = nullptr;
  memory = static_cast<char*>(std::malloc(bytes));
  fid_mr* mr = nullptr;
  if (chunkPoolContext->domain) {
    int res =
        fi_mr_reg(chunkPoolContext->domain, memory, bytes,
                  FI_RECV | FI_SEND | FI_REMOTE_READ, 0, 0, 0, &mr, nullptr);
    if (res) {
      perror("fi_mr_reg");
      return nullptr;
    }
  }
  auto first = reinterpret_cast<Chunk*>(memory);
  auto memory_ptr = first;
  for (int i = 0; i < buffer_num; i++) {
    chunkPoolContext->chunk_to_id_map[memory_ptr] = chunkPoolContext->id;
    chunkPoolContext->id_to_chunk_map[chunkPoolContext->id] =
        std::pair<Chunk*, fid_mr*>(memory_ptr, mr);
    chunkPoolContext->id++;
    memory_ptr = reinterpret_cast<Chunk*>(reinterpret_cast<char*>(memory_ptr) +
                                          chunk_size);
  }
  return reinterpret_cast<char*>(first);
}

void PoolAllocator::free(char* const block) {
  auto memory = reinterpret_cast<Chunk*>(block);
  std::free(memory);
}

void* PoolAllocator::update_context(ChunkPoolContext* chunkPoolContext_,
                                    std::function<void*()> func) {
  chunkPoolContext = chunkPoolContext_;
  return func();
}

std::mutex PoolAllocator::mtx;
ChunkPoolContext* PoolAllocator::chunkPoolContext = nullptr;

ChunkPool::ChunkPool(FabricService* service, const int request_buffer_size,
                     const int next_request_buffer_number)
    : pool(request_buffer_size + sizeof(Chunk), next_request_buffer_number, 0),
      buffer_size(request_buffer_size),
      used_buffers(0) {
  chunkPoolContext.buffer_size = buffer_size;
  chunkPoolContext.domain = nullptr;
  if (service) {
    chunkPoolContext.domain = service->get_domain();
  }
  chunkPoolContext.id = 0;
}

ChunkPool::~ChunkPool() {
  for (auto ck : chunkPoolContext.id_to_chunk_map) {
    if (ck.second.second) {
      fi_close(&((fid_mr*)ck.second.second)->fid);
    }
  }
  chunkPoolContext.id_to_chunk_map.clear();
  chunkPoolContext.chunk_to_id_map.clear();
}

void* ChunkPool::malloc() {
  if (!store().empty()) {
    return (store().malloc());
  }
  return system_malloc();
}

void ChunkPool::free(void* const ck) { (store().free)(ck); }

Chunk* ChunkPool::get(int id) {
  std::lock_guard<std::mutex> l(PoolAllocator::mtx);
  if (!chunkPoolContext.id_to_chunk_map.count(id)) {
    return nullptr;
  }
  return chunkPoolContext.id_to_chunk_map[id].first;
}

Chunk* ChunkPool::get(Connection* con) {
  std::lock_guard<std::mutex> l(PoolAllocator::mtx);
  auto ck = reinterpret_cast<Chunk*>(malloc());
  if (con) {
    con->log_used_chunk(ck);
  }
  used_buffers++;
  ck->capacity = buffer_size;
  ck->buffer_id = chunkPoolContext.chunk_to_id_map[ck];
  ck->mr = chunkPoolContext.id_to_chunk_map[ck->buffer_id].second;
  ck->buffer = ck->data;
  ck->size = 0;
  ck->ctx.internal[4] = ck;
  return ck;
}

void ChunkPool::reclaim(Chunk* ck, Connection* con) {
  std::lock_guard<std::mutex> l(PoolAllocator::mtx);
  con->remove_used_chunk(ck);
  pool::free(ck);
  used_buffers--;
}

int ChunkPool::free_size() { return INT_MAX; }

void* ChunkPool::system_malloc() {
  return PoolAllocator::update_context(&chunkPoolContext, [this] () -> void* {
    return boost::pool<PoolAllocator>::malloc();
  });
}
