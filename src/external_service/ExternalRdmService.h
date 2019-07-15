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

#ifndef EXTERNALRDMSERVICE_H
#define EXTERNALRDMSERVICE_H

#include <stdint.h>

class RdmStack;
class RdmConnection;
class ChunkMgr;
class Chunk;
class ExternalRdmCqDemultiplexer;

class ExternalRdmService {
 public:
  ExternalRdmService(int, bool);
  ~ExternalRdmService();
  ExternalRdmService(ExternalRdmService& service) = delete;
  ExternalRdmService& operator=(const ExternalRdmService& service) = delete;

  int init();
  RdmConnection* listen(const char*, const char*);
  RdmConnection* get_con(const char*, const char*);
  int wait_event(Chunk**, int*);

  void set_buffer(char*, uint64_t, int);

 private:
  RdmStack* stack;
  ExternalRdmCqDemultiplexer* demultiplexer;
  int buffer_num;
  bool is_server;
  ChunkMgr* bufMgr;
};

#endif
