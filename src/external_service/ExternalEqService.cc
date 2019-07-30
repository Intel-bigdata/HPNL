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

#include "core/MsgConnection.h"
#include "core/MsgStack.h"
#include "external_demultiplexer/ExternalEqDemultiplexer.h"
#include "external_service/ExternalEqService.h"

#include <iostream>

ExternalEqService::ExternalEqService(int worker_num_, int buffer_num_, bool is_server_)
    : worker_num(worker_num_), buffer_num(buffer_num_), is_server(is_server_) {
  stack = nullptr;
  eq_demultiplexer = nullptr;
  chkMgr = new ExternalChunkMgr();
}

ExternalEqService::~ExternalEqService() {
  if (stack) {
    delete stack;
    stack = nullptr;
  }
  if (eq_demultiplexer) {
    delete eq_demultiplexer;
    eq_demultiplexer = nullptr;
  }
  if (chkMgr) {
    delete chkMgr;
    chkMgr = nullptr;
  }
}

int ExternalEqService::init() {
  stack = new MsgStack(worker_num, buffer_num, is_server, true);
  assert(stack);
  if (stack->init() == -1) goto free_stack;
  eq_demultiplexer = new ExternalEqDemultiplexer(stack);
  assert(eq_demultiplexer);
  if (eq_demultiplexer->init() == -1) goto free_eq_demulti_plexer;

  return 0;

free_eq_demulti_plexer:
  if (eq_demultiplexer) {
    delete eq_demultiplexer;
    eq_demultiplexer = nullptr;
  }
free_stack:
  if (stack) {
    delete stack;
    stack = nullptr;
  }

  return -1;
}

fid_eq* ExternalEqService::accept(fi_info* info) {
  fid_eq* eq = stack->accept(info, chkMgr);
  return eq;
}

fid_eq* ExternalEqService::connect(const char* ip, const char* port) {
  fid_eq* eq = nullptr;
  if (is_server) {
    eq = (fid_eq*)stack->bind(ip, port, chkMgr);
    if (!eq) return nullptr;
    if (stack->listen()) {
      return nullptr;
    }
  } else {
    eq = stack->connect(ip, port, chkMgr);
    if (!eq) return nullptr;
  }
  return eq;
}

uint64_t ExternalEqService::reg_rma_buffer(char* buffer, uint64_t buffer_size,
                                           int buffer_id) {
  return stack->reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void ExternalEqService::unreg_rma_buffer(int buffer_id) {
  stack->unreg_rma_buffer(buffer_id);
}

Chunk* ExternalEqService::get_rma_buffer(int buffer_id) {
  return stack->get_rma_chunk(buffer_id);
}

void ExternalEqService::set_buffer(char* buffer, uint64_t size, int buffer_id) {
  auto* ck = new Chunk();
  ck->buffer = buffer;
  ck->buffer_id = buffer_id;
  ck->capacity = size;
  ck->mr = nullptr;
  chkMgr->reclaim(ck, nullptr);
}

int ExternalEqService::wait_eq_event(fi_info** info, fid_eq** eq, MsgConnection** con) {
  int ret = eq_demultiplexer->wait_event(info, eq, con);
  return ret;
}

Connection* ExternalEqService::get_connection(fid_eq* eq) {
  MsgConnection* con = stack->get_connection(&eq->fid);
  return con;
}

void ExternalEqService::reap(fid* con_id) { stack->reap(con_id); }

MsgStack* ExternalEqService::get_stack() { return stack; }

int ExternalEqService::get_worker_num() { return worker_num; }

int ExternalEqService::is_buffer_enough() { return chkMgr->free_size() >= 2*buffer_num; }

int ExternalEqService::add_eq_event(fid_eq* eq) {
  eq_demultiplexer->add_event(eq);
  return 0;
}

int ExternalEqService::delete_eq_event(fid_eq* eq) {
  eq_demultiplexer->delete_event(eq);
  return 0;
}
