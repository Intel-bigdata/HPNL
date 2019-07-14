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

#include "external_service/ExternalRdmService.h"

#include "HPNL/Connection.h"
#include "core/RdmConnection.h"
#include "external_demultiplexer/ExternalRdmCqDemultiplexer.h"
#include "core/RdmStack.h"

ExternalRdmService::ExternalRdmService(int buffer_num, bool is_server) {
  this->stack = nullptr;
  this->demultiplexer = nullptr;
  this->buffer_num = buffer_num;
  this->is_server = is_server;
  this->bufMgr = new ExternalChunkMgr();
}

ExternalRdmService::~ExternalRdmService() {
  delete this->stack;
  delete this->demultiplexer;
  delete this->bufMgr;
}

int ExternalRdmService::init() {
  this->stack = new RdmStack(this->buffer_num, this->is_server, true);
  this->stack->init();
  this->demultiplexer = new ExternalRdmCqDemultiplexer(stack);
  this->demultiplexer->init();
  return 0;
}

RdmConnection* ExternalRdmService::listen(const char* ip, const char* port) {
  return (RdmConnection*)stack->bind(ip, port, bufMgr);
}

RdmConnection* ExternalRdmService::get_con(const char* ip, const char* port) {
  RdmConnection *con = ((RdmStack*)stack)->get_con(ip, port, bufMgr);
  return (RdmConnection*)con;
}

int ExternalRdmService::wait_event(Chunk **ck, int *block_buffer_size) {
  return this->demultiplexer->wait_event(ck, block_buffer_size);
}

void ExternalRdmService::set_buffer(char* buffer, uint64_t size, int buffer_id) {
  Chunk *ck = new Chunk();
  ck->buffer = buffer;
  ck->buffer_id = buffer_id;
  ck->capacity = size;
  bufMgr->reclaim(ck, nullptr);
}

