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

#ifndef EQHANDLER_H
#define EQHANDLER_H

#include <rdma/fi_cm.h>

#include "HPNL/Callback.h"
#include "demultiplexer/EventHandler.h"

class Proactor;
class MsgStack;

class EqHandler : public EventHandler {
 public:
  EqHandler(MsgStack*, Proactor*, fid_eq*);
  ~EqHandler() override = default;
  int handle_event(EventType, void*) override;
  fid_eq* get_handle() const override;

  void set_accept_request_callback(Callback*) override;
  void set_connected_callback(Callback*) override;
  void set_shutdown_callback(Callback*) override;
  void set_send_callback(Callback*) override;
  void set_recv_callback(Callback*) override;
  void set_read_callback(Callback*) override;
  void set_write_callback(Callback*) override;

 private:
  MsgStack* stack;
  Proactor* proactor;
  fid_eq* eq;
  Callback* recvCallback;
  Callback* sendCallback;
  Callback* readCallback{};
  Callback* writeCallback{};
  Callback* acceptRequestCallback;
  Callback* connectedCallback;
  Callback* shutdownCallback;
};

#endif
