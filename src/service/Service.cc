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

#include "Service.h"
#include "core/ConnectionImpl.h"
#include "core/MsgConnection.h"
#include "core/MsgStack.h"
#include "core/RdmConnection.h"
#include "core/RdmStack.h"
#include "core/Stack.h"
#include "demultiplexer/CqDemultiplexer.h"
#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/EqHandler.h"
#include "demultiplexer/Proactor.h"
#include "demultiplexer/RdmCqDemultiplexer.h"

Service::Service(int worker_num_, int buffer_num_, bool is_server_)
    : worker_num(worker_num_), buffer_num(buffer_num_), is_server(is_server_), msg(true) {
  stack = nullptr;
  proactor = nullptr;
  eq_demultiplexer = nullptr;
  rdm_cq_demultiplexer = nullptr;
  eqThread = nullptr;
  rdmCqThread = nullptr;
  recvCallback = nullptr;
  sendCallback = nullptr;
  readCallback = nullptr;
  acceptRequestCallback = nullptr;
  connectedCallback = nullptr;
  shutdownCallback = nullptr;
}

Service::~Service() {
  delete stack;
  delete eq_demultiplexer;
  for (int i = 0; i < worker_num; i++) {
    delete cq_demultiplexer[i];
    if (!is_server) {
      break;
    }
  }
  delete rdm_cq_demultiplexer;
  delete proactor;
  delete acceptRequestCallback;
  delete eqThread;
  for (int i = 0; i < worker_num; i++) {
    delete cqThread[i];
    if (!is_server) {
      break;
    }
  }
}

int Service::init(bool msg_) {
  int res = 0;
  msg = msg_;
  if (msg) {
    stack = new MsgStack(worker_num, buffer_num, is_server, false);
    if ((res = stack->init())) {
      return res;
    }
    eq_demultiplexer = new EqDemultiplexer(dynamic_cast<MsgStack*>(stack));
    if ((res = eq_demultiplexer->init())) {
      return res;
    }
    for (int i = 0; i < worker_num; i++) {
      cq_demultiplexer[i] = new CqDemultiplexer(dynamic_cast<MsgStack*>(stack), i);
      if ((res = cq_demultiplexer[i]->init())) {
        return res;
      }
    }
    proactor = new Proactor(eq_demultiplexer, cq_demultiplexer, worker_num);
  } else {
    stack = new RdmStack(buffer_num, is_server, false);
    if ((res = stack->init())) {
      return res;
    }
    rdm_cq_demultiplexer = new RdmCqDemultiplexer(dynamic_cast<RdmStack*>(stack));
    if ((res = rdm_cq_demultiplexer->init())) {
      return res;
    }
    proactor = new Proactor(rdm_cq_demultiplexer);
  }

  return 0;
}

void Service::start() {
  if (proactor == nullptr) {
    return;
  }
  if (msg) {
    eqThread = new EqThread(proactor);
    eqThread->start();
    for (int i = 0; i < worker_num; i++) {
      cqThread[i] = new CqThread(proactor, i);
      cqThread[i]->start();
    }
  } else {
    rdmCqThread = new RdmCqThread(proactor);
    rdmCqThread->start();
  }
}

int Service::listen(const char* addr, const char* port) {
  if (!is_server) {
    return -1;
  }
  int res = 0;
  if (msg) {
    auto eq = (fid_eq*)stack->bind(addr, port, bufMgr);
    if (!eq) return -1;
    if ((res = ((MsgStack*)stack)->listen())) {
      return res;
    }
    std::shared_ptr<EqHandler> handler(
        new EqHandler(dynamic_cast<MsgStack*>(stack), proactor, eq));
    acceptRequestCallback = new AcceptRequestCallback(this);
    handler->set_recv_callback(recvCallback);
    handler->set_send_callback(sendCallback);
    handler->set_read_callback(readCallback);
    handler->set_accept_request_callback(acceptRequestCallback);
    handler->set_connected_callback(connectedCallback);
    handler->set_shutdown_callback(shutdownCallback);
    if (!proactor) return -1;
    if ((res = proactor->register_handler(handler))) {
      return res;
    }
  } else {
    auto con = reinterpret_cast<RdmConnection*>(stack->bind(addr, port, bufMgr));
    if (!con) return -1;
    con->set_recv_callback(recvCallback);
    con->set_send_callback(sendCallback);
  }
  return 0;
}

int Service::connect(const char* addr, const char* port) {
  if (is_server) return -1;
  int res = 0;
  fid_eq* eq = dynamic_cast<MsgStack*>(stack)->connect(addr, port, bufMgr);
  if (!eq) return -1;
  std::shared_ptr<EventHandler> handler(
      new EqHandler(dynamic_cast<MsgStack*>(stack), proactor, eq));
  acceptRequestCallback = new AcceptRequestCallback(this);
  handler->set_recv_callback(recvCallback);
  handler->set_send_callback(sendCallback);
  handler->set_read_callback(readCallback);
  handler->set_accept_request_callback(acceptRequestCallback);
  handler->set_connected_callback(connectedCallback);
  handler->set_shutdown_callback(shutdownCallback);
  if (!proactor) return -1;
  if ((res = proactor->register_handler(handler))) {
    return res;
  }
  return 0;
}

RdmConnection* Service::get_con(const char* addr, const char* port) {
  if (is_server) {
    return nullptr;
  }
  RdmConnection* con = dynamic_cast<RdmStack*>(stack)->get_con(addr, port, bufMgr);
  if (!con) {
    return nullptr;
  }
  con->set_recv_callback(recvCallback);
  con->set_send_callback(sendCallback);
  return dynamic_cast<RdmConnection*>(con);
}

void Service::shutdown() {
  for (int i = 0; i < worker_num; i++) {
    if (cqThread[i]) {
      cqThread[i]->stop();
      cqThread[i]->join();
    }
  }
  if (eqThread) {
    eqThread->stop();
    eqThread->join();
  }
  if (rdmCqThread) {
    rdmCqThread->stop();
    rdmCqThread->join();
  }
}

void Service::shutdown(Connection* con) {
  MsgConnection* msgCon = dynamic_cast<MsgConnection*>(con);
  proactor->remove_handler(msgCon->get_fid());
  msgCon->shutdown();
  dynamic_cast<MsgStack*>(stack)->reap(msgCon->get_fid());
  if (shutdownCallback) {
    shutdownCallback->operator()(nullptr, nullptr);
  }
  delete msgCon;
}

void Service::wait() {
  if (eqThread) {
    eqThread->join();
  }
  if (rdmCqThread) {
    rdmCqThread->join();
  }
}

void Service::set_buf_mgr(ChunkMgr* bufMgr_) { this->bufMgr = bufMgr_; }

void Service::set_recv_callback(Callback* callback) { recvCallback = callback; }

void Service::set_send_callback(Callback* callback) { sendCallback = callback; }

void Service::set_read_callback(Callback* callback) { readCallback = callback; }

void Service::set_connected_callback(Callback* callback) { connectedCallback = callback; }

void Service::set_shutdown_callback(Callback* callback) { shutdownCallback = callback; }

uint64_t Service::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  return dynamic_cast<MsgStack*>(stack)->reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void Service::unreg_rma_buffer(int buffer_id) {
  dynamic_cast<MsgStack*>(stack)->unreg_rma_buffer(buffer_id);
}

Chunk* Service::get_rma_buffer(int buffer_id) {
  return dynamic_cast<MsgStack*>(stack)->get_rma_chunk(buffer_id);
}

fid_domain* Service::get_domain() { return stack->get_domain(); }
