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

#include "HPNL/Callback.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Client.h"
#include "HPNL/Connection.h"
#include "service/Service.h"

Client::Client(int worker_num, int buffer_num) {
  service = new Service(worker_num, buffer_num, false);
}

Client::~Client() { delete service; }

int Client::init(bool msg) { return service->init(msg); }

void Client::start() { return service->start(); }

int Client::connect(const char* ip_, const char* port_) {
  return service->connect(ip_, port_);
}

void Client::shutdown() { service->shutdown(); }

void Client::shutdown(Connection* con) { service->shutdown(con); }

Connection* Client::get_con(const char* addr, const char* port) {
  return (Connection*)service->get_con(addr, port);
}

void Client::wait() { service->wait(); }

void Client::set_chunk_mgr(ChunkMgr* chunkMgr) { service->set_chunk_mgr(chunkMgr); }

void Client::set_send_callback(Callback* callback) {
  service->set_send_callback(callback);
}

void Client::set_recv_callback(Callback* callback) {
  service->set_recv_callback(callback);
}

void Client::set_read_callback(Callback* callback) {
  service->set_read_callback(callback);
}

void Client::set_write_callback(Callback* callback) {
  service->set_write_callback(callback);
}

void Client::set_connected_callback(Callback* callback) {
  service->set_connected_callback(callback);
}

void Client::set_shutdown_callback(Callback* callback) {
  service->set_shutdown_callback(callback);
}

Chunk* Client::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  return service->reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void Client::unreg_rma_buffer(int buffer_id) { service->unreg_rma_buffer(buffer_id); }

fid_domain* Client::get_domain() { return service->get_domain(); }
