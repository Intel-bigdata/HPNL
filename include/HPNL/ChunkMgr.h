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

#ifndef BUFMGR_H
#define BUFMGR_H

#include <rdma/fabric.h>

#include <memory>
#include <vector>
#include <map>
#include <mutex>

#include <boost/pool/pool.hpp>

#include "FabricService.h"

class Connection;

struct Chunk {
    /// Buffer unique id
    uint64_t buffer_id = 0;
    /// The number of bytes that allocated in total
    uint64_t capacity = 0;
    /// The number of bytes that are valid
    uint64_t size = 0;
    /// A pointer to HPNL connection object
    void *con = nullptr;
    /// A pointer to RDMA memory_region
    void *mr = nullptr;
    /// Libfabric context in chunk lifetime
    fi_context2 ctx{};
    /// Peer endpoint's address when sending message
    fi_addr_t peer_addr{};
    /// A pointer to a piece of contiguous memory.
    /// Chunk class won't handle the lifetime of data
    void *buffer = nullptr;
    char data[0];
};

/// Chunk manger is to manage free buffer list
/// All the function is not thread safe. That means, user application need to handle the race condition.
class ChunkMgr {
  public:
    /// Get one chunk from chunk manager
    virtual ~ChunkMgr() = default;

    /// Get one chunk
    /// \return return nullptr if no chunk in chunk manager
    virtual Chunk* get(Connection* con) = 0;

    /// Get one chunk where buffer id equals to the given buffer id
    /// \return return nullptr if no chunk in chunk manager
    virtual Chunk* get(int buffer_id) = 0;

    /// Put one chunk to chunk manager with the given buffer id
    virtual void reclaim(Chunk* chunk, Connection* con) = 0;

    /// Just Java interface need this function.
    /// For C++ interface, just return INT_MAX
    virtual int free_size() = 0;
};

/// Chunk pool allocator, reimplementing default allocator of boost pool
class PoolAllocator {
  public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    static char* malloc(size_type bytes);
    static void free(char* block);

    static int buffer_size;
    static fid_domain *domain;
    static fid_mr *mr;
    static int id;
    static std::map<Chunk*, int> chunk_to_id_map;
    static std::map<int, Chunk*> id_to_chunk_map;
    static std::mutex mtx;
};

/// HPNL provides the default chunk manager implementation, but don't want to prevent HPNL from
/// doing optimizations. User
/// can implement specific buffer manager.
/// The default ChunkMgr use boost memory pool to manage free buffer list.
class ChunkPool : public boost::pool<PoolAllocator>, public ChunkMgr {
  public:
    ChunkPool(FabricService*, int request_buffer_size,
              int next_request_buffer_number);
    ~ChunkPool() override;
    void *malloc();
    void free(void * const ck);
    Chunk* get(int id) override;
    void reclaim(Chunk* ck, Connection* con) override;
    Chunk* get(Connection* con) override;
    int free_size() override;
  private:
    void *system_malloc();
  private:
    uint64_t buffer_size;
    uint64_t used_buffers;
};

/// ExternalChunkMgr is for HPNL java interface.
class ExternalChunkMgr : public ChunkMgr {
  public:
    ExternalChunkMgr();
    ExternalChunkMgr(int buffer_num_, uint64_t buffer_size_);
    ~ExternalChunkMgr() override;
    Chunk* get(int id) override;
    void reclaim(Chunk* ck, Connection* con) override;
    Chunk* get(Connection* con) override;
    int free_size() override;
  protected:
    uint32_t get_id();
  private:
    std::mutex mtx;
    std::vector<Chunk*> bufs;
    std::map<int, Chunk*> buf_map;
    int buffer_num;
    int buffer_size;
    uint32_t buffer_id;
};

#endif
