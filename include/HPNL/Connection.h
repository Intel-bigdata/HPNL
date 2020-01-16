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

#ifndef HPNL_CONNECTION_H_
#define HPNL_CONNECTION_H_

#include <iostream>

#include "HPNL/Callback.h"
#include "HPNL/ChunkMgr.h"

class Connection {
 public:
  /// Need to call initialize connection after constructor
  virtual int init() = 0;

  /// Shutdown this connection
  /// \return return 0 if on success and return -1 if on error.
  virtual int shutdown() = 0;

  /// RDMA MSG interface
  /// \param if the chunk get from chunk pool, the buffer in chunk should be
  /// registered as RDMA buffer
  ///        if buffer need to be a RDMA buffer depends on which Libfabric
  ///        provider you're using.
  virtual int send(Chunk *ck) = 0;

  /// RDMA RMA Interface
  /// Both local and remote buffer need to be previously registered as RDMA
  /// buffer before using remote memory access semantics. \return return 0 on
  /// success and return -1 on error
  virtual int read(Chunk *ck, int local_buffer_offset,
                   uint64_t local_buffer_length, uint64_t remote_buffer_address,
                   uint64_t remote_buffer_rkey) = 0;

  /// RDMA RMA Interface
  /// Both local and remote buffer need to be previously registered as RDMA
  /// buffer before using remote memory access semantics. \return return 0 on
  /// success and return -1 on error
  virtual int write(Chunk *ck, int local_buffer_offset,
                    uint64_t local_buffer_length,
                    uint64_t remote_buffer_address,
                    uint64_t remote_buffer_rkey) = 0;

  /// For non-connection endpoint
  /// \return This function return the address of peer endpoint.
  virtual char *get_peer_name() = 0;

  /// For non-connection endpoint
  /// The structure of buffer is
  /// struct buffer {
  ///     int peer_name_length;
  ///     char* peer_name;
  ///     char* buffer;
  /// }
  /// \param ck encode given buffer to a chunk
  /// \param buffer a pointer to a piece of contiguous memory
  /// \param buffer_length the number of bytes that are valid
  /// \param peer_name peer endpoint address
  virtual void encode_(Chunk *ck, void *buffer, int buffer_length,
                       char *peer_name) = 0;

  /// For non-connection endpoint
  /// \param ck encode given buffer to a chunk
  /// \param buffer a pointer to a piece of contiguous memory
  /// \param buffer_length the number of bytes that are valid
  /// \param peer_name peer endpoint address
  virtual void decode_(Chunk *ck, void *buffer, int *buffer_length,
                       char *peer_name) = 0;

  /// Call this function when user activate recv chunk
  virtual void log_used_chunk(Chunk *ck) = 0;

  /// Call this function when user no longer need the chunk
  virtual void remove_used_chunk(Chunk *ck) = 0;
};

#endif  // HPNL_CONNECTION_H_
