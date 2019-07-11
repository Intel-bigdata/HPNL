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

#ifndef MSGSTACK_H
#define MSGSTACK_H

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_cm.h>

#include <map>
#include <assert.h>
#include <mutex>

#include "HPNL/ChunkMgr.h"
#include "HPNL/Common.h"
#include "core/Stack.h"

class MsgConnection;

class MsgStack : public Stack {
  public:
    MsgStack(int, int, bool, bool);
    ~MsgStack() override;
    MsgStack(const MsgStack& stack) = delete;
    MsgStack& operator=(const MsgStack& stack) = delete;

    // not thread safe
    int init() override;
    void* bind(const char*, const char*, ChunkMgr*) override;
    int listen();
    fid_eq* connect(const char*, const char*, ChunkMgr*);
    fid_eq* accept(void*, ChunkMgr*);

    // thread safe
    uint64_t reg_rma_buffer(char*, uint64_t, int);
    void unreg_rma_buffer(int);
    Chunk* get_rma_chunk(int);

    void reap(void*);
    MsgConnection* get_connection(fid* id);
    fid_fabric* get_fabric();
    fid_cq** get_cqs();

    fid_domain* get_domain();
  private:
    int worker_num;
    int buffer_num;
    bool is_server;
    bool external_ervice;
    uint64_t seq_num;
    fid_fabric *fabric;
    fid_domain *domain;
    fi_info *hints, *info;
    fi_info *hints_tmp, *info_tmp;
    fid_eq *peq;
    fid_pep *pep;

    std::map<fid*, MsgConnection*> conMap;

    fid_cq *cqs[MAX_WORKERS]{};

    std::map<int, Chunk*> chunkMap;
    std::mutex mtx;
    
    bool initialized;
};

#endif
