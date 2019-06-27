#include "core/RdmConnection.h"
#include <iostream>
#include <assert.h>
#include <algorithm>

RdmConnection::RdmConnection(const char* ip_, const char* port_, 
  fi_info* info_, fid_domain* domain_, fid_cq* cq_, 
  BufMgr* buf_mgr_, int buffer_num_, bool is_server_) : 
  ip(ip_), port(port_), info(info_), domain(domain_), conCq(cq_), 
  buf_mgr(buf_mgr_), buffer_num(buffer_num_), is_server(is_server_) {
  send_callback = nullptr;
  recv_callback = nullptr;
  av = nullptr;
  ep = nullptr;
 }

RdmConnection::~RdmConnection() {
  for (auto ck: send_chunks) {
    buf_mgr->put(ck->buffer_id, ck);
  }
  for (auto ck: recv_chunks) {
    buf_mgr->put(ck->buffer_id, ck);
  }
  if (!is_server) {
    fi_freeinfo(info);
  }
  fi_close(&av->fid);
  fi_close(&ep->fid);
}

int RdmConnection::init() {
  if (!is_server) {
    fi_info *hints = fi_allocinfo();
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
    if (fi_getinfo(FI_VERSION(1, 5), ip, port, is_server ? FI_SOURCE : 0, hints, &info))
      perror("fi_getinfo");
    fi_freeinfo(hints);
  }

  if (fi_endpoint(domain, info, &ep, nullptr))
    perror("fi_endpoint");
  
  fi_av_attr	av_attr;
  memset(&av_attr, 0, sizeof(av_attr));
  av_attr.type = FI_AV_UNSPEC;
  if (fi_av_open(domain, &av_attr, &av, nullptr))
    perror("fi_av_open");
  
  if (fi_ep_bind(ep, &conCq->fid, FI_SEND | FI_RECV))
    perror("fi_ep_bind cq");

  if (fi_ep_bind(ep, (fid_t)av, 0))
    perror("fi_ep_bind av");

  if (fi_enable(ep))
    perror("fi_enable");

  fi_getname((fid_t)ep, local_name, &local_name_len);

  if (!is_server) {
    char tmp[32];
    size_t tmp_len = 32;
    fi_av_straddr(av, info->dest_addr, tmp, &tmp_len);

    fi_addr_t addr;
    assert(fi_av_insert(av, info->dest_addr, 1, &addr, 0, nullptr) == 1);
    addr_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
  }

  int size = 0;
  while (size < buffer_num) {
    if (buf_mgr->free_size()) {
      Chunk *rck = buf_mgr->get();
      rck->con = this;
      rck->ctx.internal[4] = rck;
      if (fi_recv(ep, rck->buffer, rck->capacity, nullptr, FI_ADDR_UNSPEC, &rck->ctx)) {
        perror("fi_recv");
      }
      recv_chunks.push_back(rck);
    }
    if (buf_mgr->free_size()) {
      Chunk *sck = buf_mgr->get();
      send_chunks.push_back(sck);
      send_chunks_map.insert(std::pair<int, Chunk*>(sck->buffer_id, sck));
    }
    size++;
  }
  return 0;
}

int RdmConnection::shutdown() {
  // TODO
  return 0;
}

int RdmConnection::send(Chunk *ck) {
  if (fi_send(ep, ck->buffer, ck->size, nullptr, ck->peer_addr, &ck->ctx) < 0) {
    perror("fi_send");
  }
  return 0;
}

int RdmConnection::send(int buffer_size, int buffer_id) {
  Chunk *ck = send_chunks_map[buffer_id];
  char tmp[32];
  size_t tmp_len = 32;
  fi_av_straddr(av, info->dest_addr, tmp, &tmp_len);

  ck->con = this;
  ck->peer_addr = addr_map[tmp];
  ck->ctx.internal[4] = ck;

  iovec msg_iov;
  msg_iov.iov_base = ck->buffer;
  msg_iov.iov_len = buffer_size;
  fi_msg msg;
  msg.msg_iov = &msg_iov;
  msg.desc = NULL;
  msg.iov_count = 1;
  msg.addr =  ck->peer_addr;
  msg.context = &ck->ctx;
  if (fi_sendmsg(ep, &msg, FI_INJECT_COMPLETE)) {
    perror("fi_sendmsg");
    return -1;
  }
  //add_chunk_in_flight(ck);
  return 0;
}

int RdmConnection::sendBuf(const char* buffer, int buffer_size) {
  auto *ctx = (fi_context2*)std::malloc(sizeof(fi_context2));
  ctx->internal[4] = nullptr;

  char tmp[32];
  size_t tmp_len = 32;
  fi_av_straddr(av, info->dest_addr, tmp, &tmp_len);
  if (fi_send(ep, buffer, buffer_size, nullptr, addr_map[tmp], ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int RdmConnection::sendTo(int buffer_size, int buffer_id, const char* peer_name) {
  Chunk *ck = send_chunks_map[buffer_id];
  char tmp[32];
  size_t tmp_len = 32;
  fi_av_straddr(av, peer_name, tmp, &tmp_len);

  std::map<std::string, fi_addr_t>::const_iterator iter = addr_map.find(tmp);
  if (iter == addr_map.end()) {
    fi_addr_t addr;
    assert(fi_av_insert(av, peer_name, 1, &addr, 0, nullptr) == 1);
    addr_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
    ck->peer_addr = addr;
  } else {
    ck->peer_addr = addr_map[tmp];
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
  msg.addr =  ck->peer_addr;
  msg.context = &ck->ctx;
  if (fi_sendmsg(ep, &msg, FI_INJECT_COMPLETE)) {
    perror("fi_sendmsg");
    return -1;
  }

  //add_chunk_in_flight(ck);
  return 0;
}

int RdmConnection::sendBufTo(const char* buffer, int buffer_size, const char* peer_name) {
  auto *ctx = (fi_context2*)std::malloc(sizeof(fi_context2));
  ctx->internal[4] = nullptr;

  char tmp[32];
  size_t tmp_len = 32;

  fi_av_straddr(av, peer_name, tmp, &tmp_len);
  fi_addr_t peer_addr;
  std::map<std::string, fi_addr_t>::const_iterator iter = addr_map.find(tmp);
  if (iter == addr_map.end()) {
    fi_addr_t addr;
    assert(fi_av_insert(av, peer_name, 1, &addr, 0, nullptr) == 1);
    addr_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
    peer_addr = addr;
  } else {
    peer_addr = addr_map[tmp];
  }

  if (fi_send(ep, buffer, buffer_size, nullptr, peer_addr, ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}


char* RdmConnection::get_peer_name() {
  return (char*)info->dest_addr;
}

char* RdmConnection::get_local_name() {
  return local_name;
}

int RdmConnection::get_local_name_length() {
  return (int)local_name_len;
}

Chunk* RdmConnection::encode(void *buf, int size, char* peer_name) {
  Chunk *ck = send_chunks.back();
  send_chunks.pop_back();
  ck->ctx.internal[4] = ck;
  ck->con = this;
  if (local_name_len > ck->capacity) {
    return nullptr;
  }
  memcpy(ck->buffer, local_name, local_name_len);
  if (size > ck->capacity-local_name_len) {
    return nullptr;
  }
  memcpy((char*)(ck->buffer)+local_name_len, buf, size);
  ck->size = size+local_name_len;

  char tmp[32];
  size_t tmp_len = 32;
  fi_av_straddr(av, peer_name, tmp, &tmp_len);

  std::map<std::string, fi_addr_t>::const_iterator iter = addr_map.find(tmp);
  if (iter == addr_map.end()) {
    fi_addr_t address;
    assert(fi_av_insert(av, peer_name, 1, &address, 0, nullptr) == 1);
    addr_map.insert(std::pair<std::string, fi_addr_t>(tmp, address));
    ck->peer_addr = address;
  } else {
    ck->peer_addr = addr_map[tmp];
  }

  return ck;
}

void RdmConnection::decode_peer_name(void *buf, char* peer_name, int peer_name_len) {
  assert(local_name_len <= peer_name_len);
  memcpy(peer_name, buf, local_name_len);
}

char* RdmConnection::decode_buf(void *buf) {
  return (char*)buf+local_name_len;
}

int RdmConnection::activate_recv_chunk(Chunk *ck) {
  ck->con = this;
  ck->ctx.internal[4] = ck;
  if (fi_recv(ep, ck->buffer, ck->capacity, nullptr, FI_ADDR_UNSPEC, &ck->ctx)) {
    perror("fi_recv");
    return -1;
  }
  return 0;
}

void RdmConnection::activate_send_chunk(Chunk *ck) {
  send_chunks.push_back(ck);
}

std::vector<Chunk*> RdmConnection::get_send_chunk() {
  return send_chunks;
}

void RdmConnection::set_recv_callback(Callback *callback) {
  recv_callback = callback;
}

void RdmConnection::set_send_callback(Callback *callback) {
  send_callback = callback;
}

Callback* RdmConnection::get_recv_callback() {
  return recv_callback;
}

Callback* RdmConnection::get_send_callback() {
  return send_callback;
}

void RdmConnection::add_chunk_in_flight(Chunk *ck) {
  std::lock_guard<std::mutex> l(in_flight_mtx);
  chunks_in_flight[ck->buffer_id] = ck;
}

void RdmConnection::delete_chunk_in_flight(Chunk *ck) {
  std::lock_guard<std::mutex> l(in_flight_mtx);
  chunks_in_flight.erase(ck->buffer_id);
}

void RdmConnection::get_chunks_in_flight(std::unordered_map<int, Chunk *> &swap_chunks) {
  std::lock_guard<std::mutex> l(in_flight_mtx);
  chunks_in_flight.swap(swap_chunks);
}

int RdmConnection::chunks_size_in_flight() {
  std::lock_guard<std::mutex> l(in_flight_mtx);
  return chunks_in_flight.size();
}
