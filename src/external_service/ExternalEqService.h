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

#ifndef EXTERNALSERVICE_H
#define EXTERNALSERVICE_H

#include <map>

#include <rdma/fi_domain.h>
#include "HPNL/ChunkMgr.h"
#include "HPNL/Connection.h"

class MsgStack;
class MsgConnection;
class ExternalChunkMgr;
class ExternalEqDemultiplexer;

class ExternalEqService {
  public:
    ExternalEqService(int, int, bool is_server_ = false);
    ~ExternalEqService();
    ExternalEqService(const ExternalEqService& service) = delete;
    ExternalEqService& operator=(const ExternalEqService& service) = delete;

    int init();
    fid_eq* connect(const char*, const char*);
    fid_eq* accept(fi_info*);
    uint64_t reg_rma_buffer(char*, uint64_t, int);
    void unreg_rma_buffer(int);
    Chunk* get_rma_buffer(int);
    void set_buffer(char*, uint64_t, int);

    int wait_eq_event(fi_info**, fid_eq**, MsgConnection**);
    int add_eq_event(fid_eq*);
    int delete_eq_event(fid_eq*);

    Connection* get_connection(fid_eq*);
    void reap(fid*);
    MsgStack* get_stack();
    int get_worker_num();
  private:
    MsgStack *stack;

    int worker_num;
    int buffer_num;
    bool is_server;

    ExternalChunkMgr *chkMgr;

    ExternalEqDemultiplexer *eq_demultiplexer;
};

#endif
