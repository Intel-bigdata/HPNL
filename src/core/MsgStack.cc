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

#include "core/MsgConnection.h"
#include "core/MsgStack.h"

#include <iostream>

MsgStack::MsgStack(int worker_num_, int buffer_num_, bool is_server_,
                   bool external_service_)
    : worker_num(worker_num_),
      seq_num(0),
      buffer_num(buffer_num_),
      is_server(is_server_),
      external_service(external_service_),
      fabric(nullptr),
      domain(nullptr),
      hints(nullptr),
      info(nullptr),
      hints_tmp(nullptr),
      info_tmp(nullptr),
      peq(nullptr),
      pep(nullptr),
      initialized(false) {}

MsgStack::~MsgStack() {
  for (auto iter : conMap) {
    delete iter.second;
  }
  conMap.clear();
  for (auto iter : rmaChunkMap) {
    if (iter.second) {
      iter.second->buffer = nullptr;
      delete iter.second;
    }
  }
  rmaChunkMap.clear();
  if (peq) {
    fi_close(&peq->fid);
    peq = nullptr;
  }
  for (int i = 0; i < worker_num; i++) {
    if (cqs[i]) {
      fi_close(&cqs[i]->fid);
      cqs[i] = nullptr;
    }
  }
  if (domain) {
    fi_close(&domain->fid);
    domain = nullptr;
  }
  if (fabric) {
    fi_close(&fabric->fid);
    fabric = nullptr;
  }
  if (hints) {
    fi_freeinfo(hints);
    hints = nullptr;
  }
  if (info) {
    fi_freeinfo(info);
    info = nullptr;
  }
  if (hints_tmp) {
    fi_freeinfo(hints_tmp);
    hints_tmp = nullptr;
  }
  if (is_server && info_tmp) {
    fi_freeinfo(info_tmp);
    info_tmp = nullptr;
  }
}

int MsgStack::init() {
  struct fi_eq_attr eq_attr = {.size = 0,
                               .flags = 0,
                               .wait_obj = FI_WAIT_UNSPEC,
                               .signaling_vector = 0,
                               .wait_set = nullptr};

  if ((hints = fi_allocinfo()) == nullptr) {
    perror("fi_allocinfo");
    goto free_hints;
  }

  hints->addr_format = FI_SOCKADDR_IN;
  hints->ep_attr->type = FI_EP_MSG;
  hints->domain_attr->mr_mode = FI_MR_BASIC;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT | FI_LOCAL_MR;
  hints->tx_attr->msg_order = FI_ORDER_SAS;
  hints->rx_attr->msg_order = FI_ORDER_SAS;
#ifdef VERBS
  hints->fabric_attr->prov_name = strdup("verbs");
#else
  hints->fabric_attr->prov_name = strdup("sockets");
#endif

  if (fi_getinfo(FI_VERSION(1, 5), nullptr, nullptr, is_server ? FI_SOURCE : 0, hints,
                 &info)) {
    perror("fi_getinfo");
    goto free_info;
  }

  if (fi_fabric(info->fabric_attr, &fabric, nullptr)) {
    perror("fi_fabric");
    goto free_fabric;
  }

  if (fi_eq_open(fabric, &eq_attr, &peq, nullptr)) {
    perror("fi_eq_open");
    goto free_eq;
  }

  if (fi_domain(fabric, info, &domain, nullptr)) {
    perror("fi_domain");
    goto free_domain;
  }

  for (int i = 0; i < worker_num; i++) {
    struct fi_cq_attr cq_attr = {.size = 0,
                                 .flags = 0,
                                 .format = FI_CQ_FORMAT_MSG,
                                 .wait_obj = FI_WAIT_FD,
                                 .signaling_vector = 0,
                                 .wait_cond = FI_CQ_COND_NONE,
                                 .wait_set = nullptr};

    if (fi_cq_open(domain, &cq_attr, &cqs[i], nullptr)) {
      perror("fi_cq_open");
      goto free_cq;
    }
  }
  initialized = true;
  return 0;

free_cq:
  for (int i = 0; i < worker_num; i++) {
    if (cqs[i]) fi_close(&cqs[i]->fid);
  }
free_domain:
  if (domain) {
    fi_close(&domain->fid);
    domain = nullptr;
  }
free_eq:
  if (peq) {
    fi_close(&peq->fid);
    peq = nullptr;
  }
free_fabric:
  if (fabric) {
    fi_close(&fabric->fid);
    fabric = nullptr;
  }
free_info:
  if (info) {
    fi_freeinfo(info);
    info = nullptr;
  }
free_hints:
  if (hints) {
    fi_freeinfo(hints);
    hints = nullptr;
  }
  initialized = false;
  return -1;
}

void* MsgStack::bind(const char* ip_, const char* port_, ChunkMgr*) {
  if (!initialized || !ip_ || !port_) return nullptr;
  if ((hints_tmp = fi_allocinfo()) == nullptr) {
    perror("fi_allocinfo");
    return nullptr;
  }

  hints_tmp->addr_format = FI_SOCKADDR_IN;
  hints_tmp->ep_attr->type = FI_EP_MSG;
  hints_tmp->domain_attr->mr_mode = FI_MR_BASIC;
  hints_tmp->caps = FI_MSG;
  hints_tmp->mode = FI_CONTEXT | FI_LOCAL_MR;
  hints_tmp->tx_attr->msg_order = FI_ORDER_SAS;
  hints_tmp->rx_attr->msg_order = FI_ORDER_SAS;
#ifdef VERBS
  hints_tmp->fabric_attr->prov_name = strdup("verbs");
#else
  hints_tmp->fabric_attr->prov_name = strdup("sockets");
#endif

  if (fi_getinfo(FI_VERSION(1, 5), ip_, port_, is_server ? FI_SOURCE : 0, hints_tmp,
                 &info_tmp)) {
    perror("fi_getinfo");
    return nullptr;
  }

  if (fi_passive_ep(fabric, info_tmp, &pep, nullptr)) {
    perror("fi_passive_ep");
    return nullptr;
  }

  if (fi_pep_bind(pep, &peq->fid, 0)) {
    perror("fi_pep_bind");
    return nullptr;
  }
  return peq;
}

int MsgStack::listen() {
  if (!initialized || !pep) return -1;
  if (fi_listen(pep)) {
    perror("fi_listen");
    return -1;
  }
  return 0;
}

fid_eq* MsgStack::connect(const char* ip_, const char* port_, ChunkMgr* buf_mgr) {
  if (!initialized || !ip_ || !port_ || !buf_mgr) return nullptr;
  if ((hints_tmp = fi_allocinfo()) == nullptr) {
    perror("fi_allocinfo");
    return nullptr;
  }

  hints_tmp->addr_format = FI_SOCKADDR_IN;
  hints_tmp->ep_attr->type = FI_EP_MSG;
  hints_tmp->domain_attr->mr_mode = FI_MR_BASIC;
  hints_tmp->caps = FI_MSG;
  hints_tmp->mode = FI_CONTEXT | FI_LOCAL_MR;
  hints_tmp->tx_attr->msg_order = FI_ORDER_SAS;
  hints_tmp->rx_attr->msg_order = FI_ORDER_SAS;
#ifdef VERBS
  hints_tmp->fabric_attr->prov_name = strdup("verbs");
#else
  hints_tmp->fabric_attr->prov_name = strdup("sockets");
#endif

  if (fi_getinfo(FI_VERSION(1, 5), ip_, port_, is_server ? FI_SOURCE : 0, hints_tmp,
                 &info_tmp)) {
    perror("fi_getinfo");
    return nullptr;
  }

  MsgConnection* con = new MsgConnection(
      this, fabric, info_tmp, domain, cqs[seq_num % worker_num], buf_mgr, false,
      buffer_num, seq_num % worker_num, external_service);
  if (con->init()) {
    delete con;
    return nullptr;
  }
  if (int res = con->connect()) {
    if (res == EAGAIN) {
      // TODO: try again
    } else {
      delete con;
      return nullptr;
    }
  }
  con->status = CONNECT_REQ;
  seq_num++;
  conMap.insert(std::pair<fid*, MsgConnection*>(con->get_fid(), con));
  return con->get_eq();
}

fid_eq* MsgStack::accept(void* info_, ChunkMgr* buf_mgr) {
  if (!initialized || !info_) return nullptr;
  MsgConnection* con = new MsgConnection(
      this, fabric, (fi_info*)info_, domain, cqs[seq_num % worker_num], buf_mgr, true,
      buffer_num, seq_num % worker_num, external_service);
  if (con->init()) {
    delete con;
    return nullptr;
  }
  con->status = ACCEPT_REQ;
  seq_num++;
  conMap.insert(std::pair<fid*, MsgConnection*>(con->get_fid(), con));
  if (con->accept()) {
    return nullptr;
  }
  return con->get_eq();
}

Chunk* MsgStack::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  if (!initialized || !buffer || buffer_size <= 0) return nullptr;
  auto* ck = new Chunk();
  ck->buffer = buffer;
  ck->capacity = buffer_size;
  ck->buffer_id = buffer_id;
  fid_mr* mr;
  if (fi_mr_reg(domain, ck->buffer, ck->capacity,
                FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr,
                NULL)) {
    delete ck;
    perror("fi_mr_reg");
    return nullptr;
  }
  ck->mr = mr;
  std::lock_guard<std::mutex> lk(mtx);
  rmaChunkMap.insert(std::pair<int, Chunk*>(buffer_id, ck));
  return ck;
}

void MsgStack::unreg_rma_buffer(int buffer_id) {
  Chunk* ck = get_rma_chunk(buffer_id);
  if (!ck) {
    return;
  }
  fi_close(&((fid_mr*)ck->mr)->fid);
  delete (rmaChunkMap[buffer_id]);
  rmaChunkMap.erase(buffer_id);
}

Chunk* MsgStack::get_rma_chunk(int buffer_id) {
  std::lock_guard<std::mutex> lk(mtx);
  return rmaChunkMap[buffer_id];
}

void MsgStack::reap(void* con_id) {
  fid* id = (fid*)con_id;
  auto iter = conMap.find(id);
  if (iter == conMap.end()) {
    assert("connection reap failure." == nullptr);
  }
  conMap.erase(iter);
}

MsgConnection* MsgStack::get_connection(fid* id) {
  if (conMap.find(id) != conMap.end()) {
    return conMap[id];
  }
  return nullptr;
}

fid_fabric* MsgStack::get_fabric() { return fabric; }

fid_cq** MsgStack::get_cqs() { return cqs; }

fid_domain* MsgStack::get_domain() { return domain; }
