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

#ifndef RDMSTACK_H
#define RDMSTACK_H

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <string.h>
#include <assert.h>

#include <thread>
#include <mutex>
#include <vector>

#include "HPNL/ChunkMgr.h"
#include "core/Stack.h"

class RdmConnection;

class RdmStack : public Stack {
  public:
    RdmStack(int, bool);
    ~RdmStack() override;
    int init() override;
    void* bind(const char*, const char*, ChunkMgr*) override;

    RdmConnection* get_con(const char*, const char*, ChunkMgr*);
    fid_fabric* get_fabric();
    fid_cq* get_cq();
  private:
    fi_info *info;
    fi_info *server_info;
    fid_fabric *fabric;
    fid_domain *domain;
    fid_cq *cq;
    int buffer_num;
    bool is_server;

    std::mutex mtx;
    std::vector<RdmConnection*> cons;

    RdmConnection *server_con;

    bool initialized;
};

#endif
