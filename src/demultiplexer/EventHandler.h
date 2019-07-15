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

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <memory>

#include "HPNL/Callback.h"
#include "demultiplexer/EventType.h"

class fid_eq;

class EventHandler {
 public:
  virtual ~EventHandler() = default;
  virtual int handle_event(EventType, void*) = 0;
  virtual fid_eq* get_handle() const = 0;
  virtual void set_accept_request_callback(Callback* callback) = 0;
  virtual void set_connected_callback(Callback* callback) = 0;
  virtual void set_shutdown_callback(Callback* callback) = 0;
  virtual void set_recv_callback(Callback* callback) = 0;
  virtual void set_send_callback(Callback* callback) = 0;
  virtual void set_read_callback(Callback* callback) = 0;
};

#endif
