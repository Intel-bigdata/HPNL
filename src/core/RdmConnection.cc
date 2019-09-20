#include "core/RdmConnection.h"
#include <iostream>
#include <assert.h>
#include <algorithm>

RdmConnection::RdmConnection(fi_info* info_,
		fid_av* av_, fi_addr_t src_provider_addr_, fid_cq* cq_,
		fid_ep* tx_, fid_ep* rx_, BufMgr* rbuf_mgr_,
		BufMgr* sbuf_mgr_, bool is_server_) : info(info_), av(av_),
		src_provider_addr(src_provider_addr_), conCq(cq_),
		tx(tx_), rx(rx_),
		rbuf_mgr(rbuf_mgr_), sbuf_mgr(sbuf_mgr_), is_server(is_server_) {
	if(is_server){
		recv_tag = TAG_CONNECTION_REQUEST;
	}else{
		recv_tag = TAG_CONNECTION_NORMAL;
	}
}

RdmConnection::~RdmConnection() {
  for (auto ck: send_buffers) {
    sbuf_mgr->put(ck->buffer_id, ck);
  }
  for (auto ck: recv_buffers) {
    rbuf_mgr->put(ck->buffer_id, ck);
  }
  send_buffers_map.erase(send_buffers_map.begin(), send_buffers_map.end());

  delete send_global_buffers_array;

  if(info){
	fi_freeinfo(info);
	info = nullptr;
  }

  if(dest_name){
	free(dest_name);
  }
}

int RdmConnection::init(int buffer_num, int recv_buffer_num, int ctx_num) {
  int size = 0;
  while (size < recv_buffer_num ) {
	Chunk *rck = rbuf_mgr->get();
	rck->con = this;
	rck->ctx_id = 0;
	rck->ctx.internal[4] = rck;
	if (fi_trecv(rx, rck->buffer, rck->capacity, NULL, src_provider_addr,
			recv_tag, TAG_IGNORE, &rck->ctx)) {
	  perror("fi_recv");
	}
	recv_buffers.push_back(rck);
	size++;
  }
  size = 0;
  while(size < buffer_num){
	  Chunk *sck = sbuf_mgr->get();
	  sck->con = this;
	  sck->ctx_id = 0;
	  send_buffers.push_back(sck);
	  send_buffers_map.insert(std::pair<int, Chunk*>(sck->buffer_id, sck));
	  size++;
  }
  size = 0;
  while(size < ctx_num){
	Chunk *ck = new Chunk();
	ck->con = this;
	ck->ctx_id = size;
	ck->ctx.internal[5] = ck;
	ck->ctx.internal[4] = NULL;
	send_global_buffers_array[size] = ck;
	size++;
  }
  init_addr();
  if (!is_server) {
	size_t tmp_len = 32;
	dest_name = (char *)malloc(64);
	fi_av_straddr(av, info->dest_addr, dest_name, &tmp_len);
	assert(fi_av_insert(av, info->dest_addr, 1, &dest_provider_addr, 0, NULL) == 1);
  }
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
  if (fi_tsend(tx, ck->buffer, ck->size, NULL, ck->peer_addr, TAG_CONNECTION_NORMAL,
		  &ck->ctx) < 0) {
    perror("fi_send");
  }
  return 0;
}

int RdmConnection::send(int buffer_size, int buffer_id) {
  Chunk *ck = send_buffers_map[buffer_id];
  ck->ctx.internal[4] = ck;
  if (fi_tsend(tx, ck->buffer, buffer_size, NULL, dest_provider_addr,
		  TAG_CONNECTION_NORMAL, &ck->ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int RdmConnection::sendRequest(int buffer_size, int buffer_id) {
  Chunk *ck = send_buffers_map[buffer_id];
  ck->ctx.internal[4] = ck;
  if (fi_tsend(tx, ck->buffer, buffer_size, NULL, dest_provider_addr,
		  TAG_CONNECTION_REQUEST, &ck->ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int RdmConnection::sendBuf(char* buffer, int buffer_id, int ctx_id, int buffer_size) {
  Chunk *ck;
  if (ctx_id < 0) {
	ck = new Chunk();
	ck->con = this;
	ck->buffer_id = buffer_id;
	ck->ctx_id = -1; //not for cache
	ck->ctx.internal[4] = NULL;
	ck->ctx.internal[5] = ck;
  } else {
	ck = send_global_buffers_array[ctx_id];
	ck->buffer_id = buffer_id;
  }

  if (fi_tsend(tx, buffer, buffer_size, NULL, dest_provider_addr,
		  TAG_CONNECTION_NORMAL, &ck->ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int RdmConnection::sendBufWithRequest(char* buffer, int buffer_id, int ctx_id, int buffer_size) {
  Chunk *ck;
  if (ctx_id < 0) {
	ck = new Chunk();
	ck->con = this;
	ck->buffer_id = buffer_id;
	ck->ctx_id = -1; //not for cache
	ck->ctx.internal[4] = NULL;
	ck->ctx.internal[5] = ck;
  } else {
	ck = send_global_buffers_array[ctx_id];
	ck->buffer_id = buffer_id;
  }

  if (fi_tsend(tx, buffer, buffer_size, NULL, dest_provider_addr,
		  TAG_CONNECTION_REQUEST, &ck->ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int RdmConnection::sendTo(int buffer_size, int buffer_id, uint64_t peer_address) {
  Chunk *ck = send_buffers_map[buffer_id];
  ck->ctx.internal[4] = ck;
  if (fi_tsend(tx, ck->buffer, buffer_size, NULL, peer_address,
		  TAG_CONNECTION_NORMAL, &ck->ctx)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int RdmConnection::sendBufTo(char* buffer, int buffer_id, int ctx_id, int buffer_size, uint64_t peer_address) {
  Chunk *ck;
  if (ctx_id < 0) {
	ck = new Chunk();
	ck->con = this;
	ck->buffer_id = buffer_id;
	ck->ctx_id = -1; //not for cache
	ck->ctx.internal[4] = NULL;
	ck->ctx.internal[5] = ck;
  } else {
	ck = send_global_buffers_array[ctx_id];
	ck->buffer_id = buffer_id;
  }

  if (fi_tsend(tx, buffer, buffer_size, NULL, peer_address,
		  TAG_CONNECTION_NORMAL, &ck->ctx)) {
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

uint64_t RdmConnection::resolve_peer_name(char* peer_name){
  char tmp[32];
  size_t tmp_len = 64;
  fi_av_straddr(av, peer_name, tmp, &tmp_len);
  fi_addr_t addr;
  assert(fi_av_insert(av, peer_name, 1, &addr, 0, NULL) == 1);
  return (uint64_t)addr;
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
  if (fi_trecv(rx, ck->buffer, ck->capacity, NULL, src_provider_addr, recv_tag, TAG_IGNORE, &ck->ctx)) {
    perror("fi_recv");
    return -1;
  }
  return 0;
}

int RdmConnection::activate_chunk(int bufferId) {
  Chunk *ck = rbuf_mgr->get(bufferId);
  return activate_chunk(ck);
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
