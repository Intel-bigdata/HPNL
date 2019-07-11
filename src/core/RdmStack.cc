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

#include "core/RdmStack.h"
#include "core/RdmConnection.h"

#include <stdio.h>
#include <iostream>

RdmStack::RdmStack(int buffer_num_, bool is_server_) : buffer_num(buffer_num_), is_server(is_server_),
  domain(nullptr), fabric(nullptr), info(nullptr), server_info(nullptr), cq(nullptr),
  server_con(nullptr), initialized(false) {}

RdmStack::~RdmStack() {
  for (auto con : cons) {
    delete con;
    con = nullptr;
  }
  if (cq) {
    fi_close(&cq->fid);
    cq = nullptr;
  }
  if (info) {
    fi_freeinfo(info);
    info = nullptr;
  }
  if (is_server && server_info) {
    fi_freeinfo(server_info);
    server_info = nullptr;
  }
  if (domain) {
    fi_close(&domain->fid);
    domain = nullptr;
  }
  if (fabric) {
    fi_close(&fabric->fid);
    fabric = nullptr;
  }
}

int RdmStack::init() {
  fi_info* hints = fi_allocinfo();
  hints->ep_attr->type = FI_EP_RDM;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT;
#ifdef PSM2
  hints->fabric_attr->prov_name = strdup("psm2");
#elif VERBS
  hints->fabric_attr->prov_name = strdup("verbs");
#else
  hints->fabric_attr->prov_name = strdup("sockets");
#endif

  if (fi_getinfo(FI_VERSION(1, 5), nullptr, nullptr, is_server ? FI_SOURCE : 0, hints, &info)) {
    fi_freeinfo(hints);
    perror("fi_getinfo");
    return -1;
  }
  fi_freeinfo(hints);

  if (fi_fabric(info->fabric_attr, &fabric, nullptr)) {
    perror("fi_fabric");
    return -1;
  }

  if (fi_domain(fabric, info, &domain, nullptr)) {
    perror("fi_domain");
    return -1;
  }

  struct fi_cq_attr cq_attr = {
    .size = 0,
    .flags = 0,
    .format = FI_CQ_FORMAT_MSG,
    .wait_obj = FI_WAIT_FD,
    .signaling_vector = 0,
    .wait_cond = FI_CQ_COND_NONE,
    .wait_set = nullptr
  };

  if (fi_cq_open(domain, &cq_attr, &cq, nullptr)) {
    perror("fi_cq_open");
    return -1;
  }
  initialized = true;
  return 0;
}

void* RdmStack::bind(const char* ip, const char* port, ChunkMgr* buf_mgr) {
  if (!initialized || !ip || !port || !buf_mgr)
    return nullptr;
  if (buf_mgr->free_size() < buffer_num*2) {
    return nullptr;
  }
  fi_info* hints = fi_allocinfo();
  hints->ep_attr->type = FI_EP_RDM;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT;
#ifdef PSM2
  hints->fabric_attr->prov_name = strdup("psm2");
#elif VERBS
  hints->fabric_attr->prov_name = strdup("verbs");
#else
  hints->fabric_attr->prov_name = strdup("sockets");
#endif

  if (fi_getinfo(FI_VERSION(1, 5), ip, port, is_server ? FI_SOURCE : 0, hints, &server_info)) {
    fi_freeinfo(hints);
    perror("fi_getinfo");
    return nullptr;
  }
  fi_freeinfo(hints);
  server_con = new RdmConnection(ip, port, server_info, domain, cq, buf_mgr, buffer_num, true);
  server_con->init();
  cons.push_back(server_con);
  return server_con;
}

RdmConnection* RdmStack::get_con(const char* ip, const char* port, ChunkMgr* buf_mgr) {
  if (!initialized || !ip || !port || !buf_mgr)
    return nullptr;
  std::lock_guard<std::mutex> lk(mtx);
  if (buf_mgr->free_size() < buffer_num*2) {
    return nullptr;
  }
  RdmConnection *con = new RdmConnection(ip, port, nullptr, domain, cq, buf_mgr, buffer_num, false);
  con->init();
  cons.push_back(con);
  return con;
}

fid_fabric* RdmStack::get_fabric() {
  return fabric;
}

fid_cq* RdmStack::get_cq() {
  return cq;
}
