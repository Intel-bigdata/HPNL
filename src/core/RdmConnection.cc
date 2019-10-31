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

#include <sys/uio.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include "core/RdmConnection.h"

RdmConnection::RdmConnection(const char* ip_, const char* port_, fi_info* info_,
                             fid_domain* domain_, fid_cq* cq_, ChunkMgr* buf_mgr_,
                             int buffer_num_, bool is_server_, bool external_service_)
    : ip(ip_),
      port(port_),
      info(info_),
      domain(domain_),
      conCq(cq_),
      chunk_mgr(buf_mgr_),
      buffer_num(buffer_num_),
      is_server(is_server_),
      external_service(external_service_) {
  send_callback = nullptr;
  recv_callback = nullptr;
  av = nullptr;
  ep = nullptr;
}

RdmConnection::~RdmConnection() {
  for (auto ck : used_chunks) {
    chunk_mgr->reclaim(ck.second, this);
  }
  if (external_service) {
    for (auto ck : send_chunks) {
      chunk_mgr->reclaim(ck, this);
    }
  }
  if (!is_server) {
    fi_freeinfo(info);
    info = nullptr;
  }
  if (ep) {
    fi_close(&ep->fid);
    ep = nullptr;
  }
  if (av) {
    fi_close(&av->fid);
    av = nullptr;
  }
}

int RdmConnection::init() {
  if (!is_server) {
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
    assert(info == nullptr);
    if (fi_getinfo(FI_VERSION(1, 5), ip, port, is_server ? FI_SOURCE : 0, hints, &info)) {
      perror("fi_getinfo");
      fi_freeinfo(hints);
      return -1;
    }
    fi_freeinfo(hints);
  }

  if (fi_endpoint(domain, info, &ep, nullptr)) {
    perror("fi_endpoint");
    return -1;
  }

  fi_av_attr av_attr;
  memset(&av_attr, 0, sizeof(av_attr));
  av_attr.type = FI_AV_UNSPEC;
  if (fi_av_open(domain, &av_attr, &av, nullptr)) {
    perror("fi_av_open");
    return -1;
  }

  if (fi_ep_bind(ep, &conCq->fid, FI_SEND | FI_RECV)) {
    perror("fi_ep_bind cq");
    return -1;
  }

  if (fi_ep_bind(ep, (fid_t)av, 0)) {
    perror("fi_ep_bind av");
    return -1;
  }

  if (fi_enable(ep)) {
    perror("fi_enable");
    return -1;
  }

  fi_getname((fid_t)ep, local_name, &local_name_len);

  if (!is_server) {
    char tmp[32];
    size_t tmp_len = 32;
    fi_av_straddr(av, info->dest_addr, tmp, &tmp_len);

    fi_addr_t addr;
    assert(fi_av_insert(av, info->dest_addr, 1, &addr, 0, nullptr) == 1);
    address_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
  }

  int size = 0;
  while (size < buffer_num) {
    if (chunk_mgr->free_size()) {
      Chunk* rck = chunk_mgr->get(this);
      rck->con = this;
      if (fi_recv(ep, rck->buffer, rck->capacity, nullptr, FI_ADDR_UNSPEC, &rck->ctx)) {
        perror("fi_recv");
        return -1;
      }
    }
    if (external_service) {
      if (chunk_mgr->free_size()) {
        Chunk* sck = chunk_mgr->get(this);
        send_chunks.push_back(sck);
        send_chunks_map.insert(std::pair<int, Chunk*>(sck->buffer_id, sck));
      }
    }
    size++;
  }
  return 0;
}

int RdmConnection::shutdown() {
  // TODO
  return 0;
}

int RdmConnection::send(Chunk* ck) {
  int res = fi_send(ep, ck->buffer, ck->size, nullptr, ck->peer_addr, &ck->ctx);
  if (res != 0 && res != -11) {
    perror("fi_send");
  }
  return res;
}

int RdmConnection::send(int buffer_size, int buffer_id) {
  Chunk* ck = send_chunks_map[buffer_id];
  char tmp[32];
  size_t tmp_len = 32;
  fi_av_straddr(av, info->dest_addr, tmp, &tmp_len);

  ck->con = this;
  ck->peer_addr = address_map[tmp];
  ck->ctx.internal[4] = ck;

  iovec msg_iov;
  msg_iov.iov_base = ck->buffer;
  msg_iov.iov_len = buffer_size;
  fi_msg msg;
  msg.msg_iov = &msg_iov;
  msg.desc = NULL;
  msg.iov_count = 1;
  msg.addr = ck->peer_addr;
  msg.context = &ck->ctx;
  int res = fi_sendmsg(ep, &msg, FI_INJECT_COMPLETE);
  if (res != 0 && res != -11) {
    perror("fi_sendmsg");
  }
  return res;
}

int RdmConnection::sendBuf(const char* buffer, int buffer_size) {
  auto* ctx = (fi_context2*)std::malloc(sizeof(fi_context2));
  ctx->internal[4] = nullptr;

  char tmp[32];
  size_t tmp_len = 32;
  fi_av_straddr(av, info->dest_addr, tmp, &tmp_len);
  int res = fi_send(ep, buffer, buffer_size, nullptr, address_map[tmp], ctx);
  if (res != 0 && res != -11) {
    perror("fi_send");
  }
  return res;
}

int RdmConnection::sendTo(int buffer_size, int buffer_id, const char* peer_name) {
  Chunk* ck = send_chunks_map[buffer_id];
  char tmp[32];
  size_t tmp_len = 32;
  fi_av_straddr(av, peer_name, tmp, &tmp_len);

  std::map<std::string, fi_addr_t>::const_iterator iter = address_map.find(tmp);
  if (iter == address_map.end()) {
    fi_addr_t addr;
    assert(fi_av_insert(av, peer_name, 1, &addr, 0, nullptr) == 1);
    address_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
    ck->peer_addr = addr;
  } else {
    ck->peer_addr = address_map[tmp];
  }
  ck->ctx.internal[4] = ck;
  ck->con = this;

  iovec msg_iov;
  msg_iov.iov_base = ck->buffer;
  msg_iov.iov_len = buffer_size;
  fi_msg msg;
  msg.msg_iov = &msg_iov;
  msg.desc = NULL;
  msg.iov_count = 1;
  msg.addr = ck->peer_addr;
  msg.context = &ck->ctx;
  int res = fi_sendmsg(ep, &msg, FI_INJECT_COMPLETE);
  if (res != 0 && res != -11) {
    perror("fi_sendmsg");
  }
  return res;
}

int RdmConnection::sendBufTo(const char* buffer, int buffer_size, const char* peer_name) {
  auto* ctx = (fi_context2*)std::malloc(sizeof(fi_context2));
  ctx->internal[4] = nullptr;

  char tmp[32];
  size_t tmp_len = 32;

  fi_av_straddr(av, peer_name, tmp, &tmp_len);
  fi_addr_t peer_addr;
  std::map<std::string, fi_addr_t>::const_iterator iter = address_map.find(tmp);
  if (iter == address_map.end()) {
    fi_addr_t addr;
    assert(fi_av_insert(av, peer_name, 1, &addr, 0, nullptr) == 1);
    address_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
    peer_addr = addr;
  } else {
    peer_addr = address_map[tmp];
  }
  int res = fi_send(ep, buffer, buffer_size, nullptr, peer_addr, ctx);
  if (res != 0 && res != -11) {
    perror("fi_send");
  }
  return res;
}

char* RdmConnection::get_peer_name() { return (char*)info->dest_addr; }

char* RdmConnection::get_local_name() { return local_name; }

int RdmConnection::get_local_name_length() { return (int)local_name_len; }

void RdmConnection::encode_(Chunk* ck, void* buffer, int buffer_size, char* peer_name) {
  ck->ctx.internal[4] = ck;
  ck->con = this;
  if (local_name_len > ck->capacity) {
    return;
  }
  memcpy(ck->buffer, local_name, local_name_len);
  if (buffer_size > ck->capacity - local_name_len) {
    return;
  }
  memcpy((char*)(ck->buffer) + local_name_len, buffer, buffer_size);
  ck->size = buffer_size + local_name_len;

  char tmp[32];
  size_t tmp_len = 32;
  fi_av_straddr(av, peer_name, tmp, &tmp_len);

  std::map<std::string, fi_addr_t>::const_iterator iter = address_map.find(tmp);
  if (iter == address_map.end()) {
    fi_addr_t address;
    assert(fi_av_insert(av, peer_name, 1, &address, 0, nullptr) == 1);
    address_map.insert(std::pair<std::string, fi_addr_t>(tmp, address));
    ck->peer_addr = address;
  } else {
    ck->peer_addr = address_map[tmp];
  }
}

void RdmConnection::decode_(Chunk* ck, void* buffer, int* buffer_size, char* peer_name) {
  if (peer_name) memcpy(peer_name, ck->buffer, local_name_len);
  if (buffer) {
    *buffer_size = ck->size - local_name_len;
    memcpy(buffer, reinterpret_cast<char*>(ck->buffer) + local_name_len, *buffer_size);
  }
}

int RdmConnection::activate_recv_chunk(Chunk* ck) {
  if (ck == nullptr) {
    ck = chunk_mgr->get(this);
  }
  ck->con = this;
  if (fi_recv(ep, ck->buffer, ck->capacity, nullptr, FI_ADDR_UNSPEC, &ck->ctx)) {
    perror("fi_recv");
    return -1;
  }
  return 0;
}

std::vector<Chunk*> RdmConnection::get_send_chunk() { return send_chunks; }

void RdmConnection::set_recv_callback(Callback* callback) { recv_callback = callback; }

void RdmConnection::set_send_callback(Callback* callback) { send_callback = callback; }

Callback* RdmConnection::get_recv_callback() { return recv_callback; }

Callback* RdmConnection::get_send_callback() { return send_callback; }
