#include "core/RdmConnection.h"
#include <iostream>
#include <assert.h>
#include <algorithm>

RdmConnection::RdmConnection(const char* ip_, const char* port_, fi_info* info_, fid_domain* domain_, fid_cq* cq_, BufMgr* rbuf_mgr_, BufMgr* sbuf_mgr_, int buffer_num_, bool is_server_, const char* prov_name_) : ip(ip_), port(port_), info(info_), domain(domain_), conCq(cq_), rbuf_mgr(rbuf_mgr_), sbuf_mgr(sbuf_mgr_), buffer_num(buffer_num_), is_server(is_server_), prov_name(prov_name_) {}
RdmConnection::~RdmConnection() {
  for (auto ck: send_buffers) {
    sbuf_mgr->put(ck->buffer_id, ck);
  }
  for (auto ck: recv_buffers) {
    rbuf_mgr->put(ck->buffer_id, ck);
  }
  if(av){
  	  fi_close(&av->fid);
  	  av = nullptr;
    }
  if(ep){
	  fi_close(&ep->fid);
	  ep = nullptr;
  }
  if (!is_server) {
	if(info){
		fi_freeinfo(info);
    	info = nullptr;
	}
  }
  prov_name = nullptr;
}

int RdmConnection::init() {
  if (!is_server) {
    fi_info *hints = fi_allocinfo();
    hints->ep_attr->type = FI_EP_RDM;
    hints->caps = FI_MSG;
    hints->mode = FI_CONTEXT;
    if (prov_name != nullptr){
      hints->fabric_attr->prov_name = strdup(prov_name);
    }

    assert(info == NULL);
    if (fi_getinfo(FI_VERSION(1, 5), ip, port, is_server ? FI_SOURCE : 0, hints, &info))
      perror("fi_getinfo");
    fi_freeinfo(hints);
  }
  if (fi_endpoint(domain, info, &ep, NULL))
    perror("fi_endpoint");
  
  fi_av_attr	av_attr;
  memset(&av_attr, 0, sizeof(av_attr));
  av_attr.type = FI_AV_UNSPEC;
  if (fi_av_open(domain, &av_attr, &av, NULL))
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
    assert(fi_av_insert(av, info->dest_addr, 1, &addr, 0, NULL) == 1);
    addr_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
  }
  int size = 0;
  while (size < 2*buffer_num) {
    Chunk *rck = rbuf_mgr->get();
    rck->con = this;
    rck->ctx.internal[4] = rck;
    if (fi_recv(ep, rck->buffer, rck->capacity, NULL, FI_ADDR_UNSPEC, &rck->ctx)) {
      perror("fi_recv");
    }
    recv_buffers.push_back(rck);
    size++;
  }
  size = 0;
  while(size < buffer_num){
	Chunk *sck = sbuf_mgr->get();
	send_buffers.push_back(sck);
	send_buffers_map.insert(std::pair<int, Chunk*>(sck->buffer_id, sck));
	size++;
  }
  init_addr();
  return 0;
}

void RdmConnection::init_addr() {}

void RdmConnection::get_addr(char** dest_addr_, size_t* dest_port_, char** src_addr_, size_t* src_port_) {
  *dest_addr_ = dest_addr;
  *dest_port_ = dest_port;

  *src_addr_ = src_addr;
  *src_port_ = src_port;
}

int RdmConnection::send(Chunk *ck) {
  if (fi_send(ep, ck->buffer, ck->size, NULL, ck->peer_addr, &ck->ctx) < 0) {
    perror("fi_send");
  }
  return 0;
}

int RdmConnection::send(int buffer_size, int buffer_id) {
  Chunk *ck = send_buffers_map[buffer_id];
  char tmp[32];
  size_t tmp_len = 64;
  fi_av_straddr(av, info->dest_addr, tmp, &tmp_len);

  ck->con = this;
  ck->peer_addr = addr_map[tmp];
  ck->ctx.internal[4] = ck;
  if (fi_send(ep, ck->buffer, buffer_size, NULL, ck->peer_addr, &ck->ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int RdmConnection::sendBuf(const char* buffer, int buffer_size) {
  fi_context2 *ctx = (fi_context2*)std::malloc(sizeof(fi_context2));
  ctx->internal[4] = NULL;

  char tmp[32];
  size_t tmp_len = 64;
  fi_av_straddr(av, info->dest_addr, tmp, &tmp_len);
  if (fi_send(ep, buffer, buffer_size, NULL, addr_map[tmp], ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int RdmConnection::sendTo(int buffer_size, int buffer_id, const char* peer_name) {
  Chunk *ck = send_buffers_map[buffer_id];
  char tmp[32];
  size_t tmp_len = 64;
  fi_av_straddr(av, peer_name, tmp, &tmp_len);

  std::map<std::string, fi_addr_t>::const_iterator iter = addr_map.find(tmp);
  if (iter == addr_map.end()) {
    fi_addr_t addr;
    assert(fi_av_insert(av, peer_name, 1, &addr, 0, NULL) == 1);
    addr_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
    ck->peer_addr = addr;
  } else {
    ck->peer_addr = addr_map[tmp];
  }
  ck->ctx.internal[4] = ck;
  ck->con = this;
  if (fi_send(ep, ck->buffer, buffer_size, NULL, ck->peer_addr, &ck->ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int RdmConnection::sendBufTo(const char* buffer, int buffer_size, const char* peer_name) {
  fi_context2 *ctx = (fi_context2*)std::malloc(sizeof(fi_context2));
  ctx->internal[4] = NULL;

  char tmp[32];
  size_t tmp_len = 64;

  fi_av_straddr(av, peer_name, tmp, &tmp_len);
  fi_addr_t peer_addr;
  std::map<std::string, fi_addr_t>::const_iterator iter = addr_map.find(tmp);
  if (iter == addr_map.end()) {
    fi_addr_t addr;
    assert(fi_av_insert(av, peer_name, 1, &addr, 0, NULL) == 1);
    addr_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
    peer_addr = addr;
  } else {
    peer_addr = addr_map[tmp];
  }

  if (fi_send(ep, buffer, buffer_size, NULL, peer_addr, ctx)) {
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
  return local_name_len;
}

Chunk* RdmConnection::encode(void *buf, int size, char* peer_name) {
  Chunk *ck = send_buffers.back();
  send_buffers.pop_back();
  ck->ctx.internal[4] = ck;
  ck->con = this;
  memcpy(ck->buffer, local_name, local_name_len);
  memcpy((char*)(ck->buffer)+local_name_len, buf, size);
  ck->size = size+local_name_len;
  char tmp[32];
  size_t tmp_len = 64;
  fi_av_straddr(av, peer_name, tmp, &tmp_len);
  std::map<std::string, fi_addr_t>::const_iterator iter = addr_map.find(tmp);
  if (iter == addr_map.end()) {
    fi_addr_t addr;
    assert(fi_av_insert(av, peer_name, 1, &addr, 0, NULL) == 1);
    addr_map.insert(std::pair<std::string, fi_addr_t>(tmp, addr));
    ck->peer_addr = addr;
  } else {
    ck->peer_addr = addr_map[tmp];
  }
  return ck;
}
void RdmConnection::decode_peer_name(void *buf, char* peer_name) {
  memcpy(peer_name, buf, local_name_len);
}
char* RdmConnection::decode_buf(void *buf) {
  return (char*)buf+local_name_len;
}
fid_cq* RdmConnection::get_cq() {
  return conCq; 
}
int RdmConnection::activate_chunk(Chunk *ck) {
  ck->con = this;
  ck->ctx.internal[4] = ck;
  if (fi_recv(ep, ck->buffer, ck->capacity, NULL, FI_ADDR_UNSPEC, &ck->ctx)) {
    perror("fi_recv");
    return -1;
  }
  return 0;
}
void RdmConnection::reclaim_chunk(Chunk *ck) {
  send_buffers.push_back(ck);
}

std::vector<Chunk*> RdmConnection::get_send_buffer() {
  return send_buffers;
}

std::vector<Chunk*> RdmConnection::get_recv_buffer(){
  return recv_buffers;
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
