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

#include "core/MsgStack.h"
#include "core/MsgConnection.h"
#include "demultiplexer/EqHandler.h"
#include "demultiplexer/Proactor.h"

EqHandler::EqHandler(MsgStack *stack_, Proactor *proactor_, fid_eq *eq_) : stack(stack_), proactor(proactor_),
  eq(eq_), recvCallback(nullptr), sendCallback(nullptr),
  acceptRequestCallback(nullptr), connectedCallback(nullptr),
  shutdownCallback(nullptr) {}

int EqHandler::handle_event(EventType et, void *context) {
  auto *entry = (fi_eq_cm_entry*)context;
  if (et == ACCEPT_EVENT) {
    assert(acceptRequestCallback);
    ChunkMgr *buf_mgr;
    (*acceptRequestCallback)(&buf_mgr, nullptr);

    fid_eq *local_eq = stack->accept(entry->info, buf_mgr);
    std::shared_ptr<EqHandler> eqHandler = std::make_shared<EqHandler>(stack, proactor, local_eq);
    if (!eqHandler)
      return -1;
    if (connectedCallback) {
      eqHandler->set_connected_callback(connectedCallback); 
    }
    if (recvCallback) {
      eqHandler->set_recv_callback(recvCallback); 
    }
    if (sendCallback) {
      eqHandler->set_send_callback(sendCallback);
    }
    if (readCallback) {
      eqHandler->set_read_callback(readCallback);
    }
    if (shutdownCallback) {
      eqHandler->set_shutdown_callback(shutdownCallback);
    }
    proactor->register_handler(eqHandler);
  } else if (et == CONNECTED_EVENT) {
    auto con = stack->get_connection(entry->fid);
    assert(con);
    if (recvCallback) {
      con->set_recv_callback(recvCallback);
    }
    if (shutdownCallback) {
      con->set_shutdown_callback(shutdownCallback);
    }
    if (sendCallback) {
      con->set_send_callback(sendCallback);
    }
    if (readCallback) {
      con->set_read_callback(readCallback);
    }
   
    {
      std::lock_guard<std::mutex> l(con->con_mtx);
      con->status = CONNECTED;
    }
    con->con_cv.notify_one();

    con->init_addr();

    if (connectedCallback) {
      (*connectedCallback)(con, nullptr);
    }
  } else if (et == CLOSE_EVENT) {
    auto con = stack->get_connection(entry->fid);
    assert(con);
    con->status = SHUTDOWN_REQ;
    if (con->get_shutdown_callback()) {
      (*(con->get_shutdown_callback()))(nullptr, nullptr);
    }
    proactor->remove_handler(&get_handle()->fid);
    con->status = DOWN;
    stack->reap(entry->fid);
  } else {
    // TODO: exception handler
  }
  return 0;
}

fid_eq* EqHandler::get_handle() const {
  return eq;
}

void EqHandler::set_accept_request_callback(Callback *callback) {
  acceptRequestCallback = callback;
}

void EqHandler::set_connected_callback(Callback *callback) {
  connectedCallback = callback;
}

void EqHandler::set_shutdown_callback(Callback *callback) {
  shutdownCallback = callback;
}

void EqHandler::set_send_callback(Callback *callback) {
  sendCallback = callback;
}

void EqHandler::set_recv_callback(Callback *callback) {
  recvCallback = callback;
}

void EqHandler::set_read_callback(Callback *callback) {
  readCallback = callback;
}



