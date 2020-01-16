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

#ifndef HPNL_CLIENT_H_
#define HPNL_CLIENT_H_

#include <cstdint>

#include "FabricService.h"

class Service;
class Connection;
class ChunkMgr;
class Callback;
class Chunk;
class fid_domain;

class Client : public FabricService {
 public:
  /// \param worker_number message handling thread.
  /// \param initial_buffer_number buffer originally allocated to transfer
  /// send/receive message.
  explicit Client(int worker_number = 1, int initial_buffer_number = 16);
  ~Client();

  /// Connection management
  /// HPNL supports connection(MSG) and non-connection(RDM) endpoints.
  ///
  /// \param msg If msg_type is set to true, client will be initialized as
  /// connection(MSG) endpoint;
  ///            otherwise, client will be initialized as non-connection(RDM)
  ///            endpoint. Default type is connection endpoint.
  /// \return 0 on success or -1 on error.
  int init(bool msg_type = true);

  /// For connection endpoint.
  int connect(const char* ip, const char* port);

  /// For connection endpoint
  /// Shutdown the server process and disconnect all the connections
  void shutdown();

  /// For connection endpoint
  /// Disconnect specified connection
  /// \param specify the connection
  void shutdown(Connection* connection);

  /// For non-connection endpoint.
  /// Actually, there's no connection for non-connection endpoint.
  /// Connection return from get_con is virtual connection.
  Connection* get_con(const char* ip, const char* port);

  /// start demultiplexer polling thread.
  void start();

  /// wait demultiplexer polling thread to stop.
  void wait();

  /// Buffer management
  /// This method should be called after init
  /// \param set chunk management to server
  void set_chunk_mgr(ChunkMgr* chunkMgr);

  /// Initialize event callback
  /// \param this function will be called when received new message.
  void set_recv_callback(Callback* callback);

  /// \param this function will be called when message has been sent.
  void set_send_callback(Callback* callback);

  /// \param this function will be called when read new message from remote
  /// node.
  void set_read_callback(Callback* callback);

  /// \param this function will be called when write new message to remote node.
  void set_write_callback(Callback* callback);

  /// \param this function will be called when a new client has been connected.
  void set_connected_callback(Callback* callback);

  /// \param this function will be called when a new client has been shutdown.
  void set_shutdown_callback(Callback* callback);

  /// RMA buffer registration
  /// \return return 0 on success and return -1 on error
  Chunk* reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id);

  /// unregister RMA buffer that buffer id is buffer_id
  void unreg_rma_buffer(int buffer_id);

  /// get RMA buffer that buffer id is buffer_id
  Chunk* get_rma_buffer(int buffer_id);

  /// For RDMA memory registration
  fid_domain* get_domain() override;

 private:
  Service* service;
};

#endif  // HPNL_CLIENT_H_
