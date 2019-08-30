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

#include <arpa/inet.h>
#include <netinet/in.h>

#include "core/MsgConnection.h"
#include "core/MsgStack.h"

#include <iostream>

MsgConnection::MsgConnection(MsgStack* stack_, fid_fabric* fabric_, fi_info* info_,
                             fid_domain* domain_, fid_cq* cq_, ChunkMgr* buf_mgr_,
                             bool is_server_, int buffer_num_, int cq_index_,
                             bool external_ervice_)
    : stack(stack_),
      fabric(fabric_),
      info(info_),
      domain(domain_),
      conCq(cq_),
      chunk_mgr(buf_mgr_),
      is_server(is_server_),
      buffer_num(buffer_num_),
      cq_index(cq_index_),
      external_ervice(external_ervice_) {
  conEq = nullptr;
  ep = nullptr;
  status = IDLE;
  dest_port = 0;
  src_port = 0;
  recv_callback = nullptr;
  send_callback = nullptr;
  read_callback = nullptr;
  shutdown_callback = nullptr;
}

MsgConnection::~MsgConnection() {
  for (auto ck : used_chunks) {
    chunk_mgr->reclaim(ck.second, this);
  }
  if (external_ervice) {
    for (auto ck : send_chunks) {
      chunk_mgr->reclaim(ck, this);
    }
  }
  if (ep) {
    shutdown();
    fi_close(&ep->fid);
    ep = nullptr;
  }
  if (conEq) {
    fi_close(&conEq->fid);
    conEq = nullptr;
  }
  if (info) {
    fi_freeinfo(info);
    info = nullptr;
  }
}

int MsgConnection::init() {
  int size = 0;
  struct fi_eq_attr eq_attr = {.size = 0,
                               .flags = 0,
                               .wait_obj = FI_WAIT_UNSPEC,
                               .signaling_vector = 0,
                               .wait_set = nullptr};

  if (fi_endpoint(domain, info, &ep, nullptr)) {
    perror("fi_endpoint");
    goto free_ep;
  }

  if (fi_eq_open(fabric, &eq_attr, &conEq, nullptr)) {
    perror("fi_eq_open");
    goto free_eq;
  }

  if (fi_ep_bind(ep, &conEq->fid, 0)) {
    perror("fi_ep_bind");
    goto free_eq;
  }

  if (fi_ep_bind(ep, &conCq->fid, FI_TRANSMIT | FI_RECV)) {
    perror("fi_ep_bind");
    goto free_eq;
  }

  fi_enable(ep);
  while (size < buffer_num) {
    Chunk* ck = chunk_mgr->get(this);
    if (!ck) {
      return -1;
    }
    if (external_ervice) {
      fid_mr* mr = nullptr;
      if (fi_mr_reg(domain, ck->buffer, ck->capacity,
                    FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr,
                    NULL)) {
        perror("fi_mr_reg");
        return -1;
      }
      ck->mr = mr;
      mr = nullptr;
    }
    ck->con = this;
    if (fi_recv(ep, ck->buffer, ck->capacity, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
      perror("fi_recv");
      return -1;
    }

    if (external_ervice) {
      ck = chunk_mgr->get(this);
      if (!ck) {
        return -1;
      }
      fid_mr* mr = nullptr;
      if (fi_mr_reg(domain, ck->buffer, ck->capacity,
                    FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr,
                    NULL)) {
        perror("fi_mr_reg");
        return -1;
      }
      ck->mr = mr;
      mr = nullptr;
      ck->con = this;
      send_chunks.push_back(ck);
    }
    size++;
  }
  return 0;

free_eq:
  if (conEq) {
    fi_close(&conEq->fid);
    conEq = nullptr;
  }
free_ep:
  if (ep) {
    fi_close(&ep->fid);
    ep = nullptr;
  }

  return -1;
}

int MsgConnection::send(Chunk* ck) {
  ck->con = this;
  if (fi_send(ep, ck->buffer, (size_t)ck->size, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int MsgConnection::send(int buffer_size, int id) {
  Chunk* ck = chunk_mgr->get(id);
  ck->size = buffer_size;
  if (ck == nullptr) return -1;
  ck->con = this;
  if (fi_send(ep, ck->buffer, (size_t)buffer_size, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int MsgConnection::read(int buffer_id, int local_offset, uint64_t len,
                        uint64_t remote_addr, uint64_t remote_key) {
  Chunk* ck = stack->get_rma_chunk(buffer_id);
  ck->con = this;
  return fi_read(ep, (char*)ck->buffer + local_offset, len, fi_mr_desc((fid_mr*)ck->mr),
                 0, remote_addr, remote_key, ck);
}

int MsgConnection::read(Chunk *ck, int local_offset, uint64_t len,
                        uint64_t remote_addr, uint64_t remote_key) {
  ck->con = this;
  return fi_read(ep, (char*)ck->buffer + local_offset, len, fi_mr_desc((fid_mr*)ck->mr),
                 0, remote_addr, remote_key, ck);
}

int MsgConnection::connect() {
  int res = fi_connect(ep, info->dest_addr, nullptr, 0);
  if (res) {
    if (res == EAGAIN) {
      return EAGAIN;
    } else {
      perror("fi_connect");
      return -1;
    }
  }
  return 0;
}

int MsgConnection::accept() {
  if (fi_accept(ep, nullptr, 0)) {
    perror("fi_accept");
    return -1;
  }
  return 0;
}

int MsgConnection::shutdown() { return fi_shutdown(ep, 0); }

void MsgConnection::init_addr() {
  if (!info->dest_addr) {
    auto dest_addr_in = (struct sockaddr_in*)info->dest_addr;
    dest_port = dest_addr_in->sin_port;
    char* addr = inet_ntoa(dest_addr_in->sin_addr);
    strcpy(dest_addr, addr);
  }

  if (!info->src_addr) {
    auto src_addr_in = (struct sockaddr_in*)info->src_addr;
    src_port = src_addr_in->sin_port;
    char* addr = inet_ntoa(src_addr_in->sin_addr);
    strcpy(src_addr, addr);
  }
}

void MsgConnection::get_addr(char** dest_addr_, size_t* dest_port_, char** src_addr_,
                             size_t* src_port_) {
  *dest_addr_ = dest_addr;
  *dest_port_ = dest_port;

  *src_addr_ = src_addr;
  *src_port_ = src_port;
}

int MsgConnection::get_cq_index() { return cq_index; }

void MsgConnection::set_recv_callback(Callback* callback) { recv_callback = callback; }

void MsgConnection::set_send_callback(Callback* callback) { send_callback = callback; }

void MsgConnection::set_read_callback(Callback* callback) { read_callback = callback; }

void MsgConnection::set_shutdown_callback(Callback* callback) {
  shutdown_callback = callback;
}

Callback* MsgConnection::get_recv_callback() { return recv_callback; }

Callback* MsgConnection::get_send_callback() { return send_callback; }

Callback* MsgConnection::get_read_callback() { return read_callback; }

Callback* MsgConnection::get_shutdown_callback() { return shutdown_callback; }

std::vector<Chunk*> MsgConnection::get_send_chunks() { return send_chunks; }

fid* MsgConnection::get_fid() { return &conEq->fid; }

int MsgConnection::activate_recv_chunk(Chunk* ck) {
  if (ck == nullptr) {
    ck = chunk_mgr->get(this);
  }
  ck->con = this;
  if (fi_recv(ep, ck->buffer, ck->capacity, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    perror("fi_recv");
    return -1;
  }
  return 0;
}

fid_eq* MsgConnection::get_eq() { return conEq; }
