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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "HPNL/ChunkMgr.h"
#include "HPNL/Callback.h"

class Connection {
  public:
    virtual ~Connection() = default;;
    virtual int init() = 0;
    virtual int shutdown() = 0;
    // Message Interface
    // For connection endpoint.
    virtual int send(Chunk* ck) { return 0; }
    virtual int send(int buffer_size, int id) { return 0; }
    virtual int sendBuf(const char* buffer, int buffer_size) { return 0; }
    // Peer_address got from get_peer_name
    virtual int sendTo(int buffer_size, int buffer_id, const char* peer_address) { return 0; }
    virtual int sendBufTo(const char* buffer, int buffer_size, const char* peer_address) { return 0; }
    virtual fi_addr_t recv(const char* buffer, int buffer_id) { return 0; }

    // For non-connection endpoint.
    virtual char* get_peer_name() { return nullptr; }
    virtual void decode_peer_name(void* buffer, char* peer_name, int peer_name_length) {}
    virtual char* decode_buf(void *buffer) { return nullptr; }
    virtual Chunk* encode(void* buffer, int size, char* peer_name) { return nullptr; }

    virtual void activate_send_chunk(Chunk* ck) {}
    // HPNL supports asynchronous event handling, and is based on the Proactor design pattern.
    // User need to post chunk to recv queue before sending/receiving message. When recv event happened,
    // event handling thread will get the chunk in the recv queue with message received from peer endpoint.
    // User need to activate the chunk that received in recv callback
    virtual int activate_recv_chunk(Chunk* ck) { return 0; }
    virtual void set_recv_callback(Callback*) {}
    virtual void set_send_callback(Callback*) {}
    virtual Callback* get_recv_callback() = 0;
    virtual Callback* get_send_callback() = 0;

    // Remote Memory Access Interface
    // Both local and remote buffer need to be previously registered as RDMA buffer
    // before using remote memory access semantics.
    virtual int read(int local_buffer_id, int local_buffer_offset, uint64_t local_buffer_length,
                     uint64_t remote_buffer_address, uint64_t remote_buffer_rkey) { return 0; }
    
    // Buffer management
    virtual void log_used_chunk(Chunk*) {}
    virtual void remove_used_chunk(Chunk*) {}
};

#endif
