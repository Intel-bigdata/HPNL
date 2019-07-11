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

#ifndef EVENTTYPE_H
#define EVENTTYPE_H

enum EventType {
  ACCEPT_EVENT = 1,
  CONNECTED_EVENT = 2,
  READ_EVENT = 4,
  WRITE_EVENT = 8,
  RECV_EVENT = 16,
  SEND_EVENT = 32,
  CLOSE_EVENT = 64,
  ERROR_EVENT = 128,
  CONNECT = 256,
  ACCEPT = 512,
  SHUTDOWN = 1024
};

#endif
