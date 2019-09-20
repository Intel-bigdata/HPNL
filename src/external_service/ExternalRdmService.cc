#include "external_service/ExternalRdmService.h"

#include "HPNL/Connection.h"
#include "core/RdmConnection.h"
#include "external_service/ExternalEqServiceBufMgr.h"
#include "core/RdmStack.h"

#include <iostream>

ExternalRdmService::ExternalRdmService(int buffer_num, int recv_buffer_num, int ctx_num, int read_batch_size, bool is_server) {
  this->buffer_num = buffer_num;
  this->recv_buffer_num = recv_buffer_num;
  this->ctx_num = ctx_num;
  this->read_batch_size = read_batch_size;
  this->is_server = is_server;
  this->recvBufMgr = new ExternalEqServiceBufMgr();
  this->sendBufMgr = new ExternalEqServiceBufMgr();
}

ExternalRdmService::~ExternalRdmService() {
  delete this->stack;
  delete this->recvBufMgr;
  delete this->sendBufMgr;
}

int ExternalRdmService::init(const char* prov_name) {
  this->stack = new RdmStack(this->buffer_num, this->recv_buffer_num, this->ctx_num, this->is_server, prov_name);
  this->stack->init();
  cqs = stack->get_cqs();
  return 0;
}

RdmConnection* ExternalRdmService::listen(const char* ip, const char* port) {
  return (RdmConnection*)stack->bind(ip, port, recvBufMgr, sendBufMgr);
}

RdmConnection* ExternalRdmService::get_con(const char* ip, const char* port, uint64_t src_provider_addr,
		int cq_index) {
  RdmConnection *con = ((RdmStack*)stack)->get_con(ip, port, src_provider_addr,
		  cq_index, recvBufMgr, sendBufMgr);
  return (RdmConnection*)con;
}

int ExternalRdmService::wait_event(int cq_index, int(*process)(Chunk *, int, int, int)) {
  fi_cq_tagged_entry entries[read_batch_size];
  int ret = fi_cq_read(cqs[cq_index], &entries, read_batch_size);
  if(ret > 0){
	for(int j=0; j<ret; j++){
		fi_cq_tagged_entry* entry = &entries[j];
		if (entry->flags & FI_RECV) {
		  fi_context2 *ctx = (fi_context2*)entry->op_context;
		  Chunk *ck =(Chunk*)ctx->internal[4];
		  int buffer_size = entry->len;
		  if(process(ck, ck->buffer_id, buffer_size, RECV_EVENT) < 0){
			  return -1;
		  }
		  continue;
		}
		if (entry->flags & FI_SEND) {
		  fi_context2 *ctx = (fi_context2*)entry->op_context;
		  Chunk *ck;
		  int buffer_size;
		  if (ctx->internal[4] != NULL) {
			  ck = (Chunk*)ctx->internal[4];
			  buffer_size = entry->len;
		  } else {
			  ck = (Chunk*)ctx->internal[5];
			  buffer_size = ck->ctx_id;
		  }
		  if(process(ck, ck->buffer_id, buffer_size, SEND_EVENT) < 0){
			  return -1;
		  }
		  continue;
		}
//		if (entry.flags & FI_READ) {
//		  return READ_EVENT;
//		}
//		if (entry.flags & FI_WRITE) {
//		  return WRITE_EVENT;
//		}
	}
	return 0;
  }

  if (ret == -FI_EAVAIL) {
	fi_cq_err_entry err_entry;
	fi_cq_readerr(cqs[cq_index], &err_entry, 0);
	perror("fi_cq_read");
	if (err_entry.err == FI_EOVERRUN) {
	  return -1;
	}
	return 0;
  }
  return 0;
}

void ExternalRdmService::reap(int64_t connection_id){
  stack->reap(connection_id);
}

void ExternalRdmService::set_recv_buffer(char* buffer, uint64_t size, int buffer_id) {
  Chunk *ck = new Chunk();
  ck->buffer = buffer;
  ck->buffer_id = buffer_id;
  ck->capacity = size;
  recvBufMgr->put(ck->buffer_id, ck);
}

void ExternalRdmService::set_send_buffer(char* buffer, uint64_t size, int buffer_id) {
  Chunk *ck = new Chunk();
  ck->buffer = buffer;
  ck->buffer_id = buffer_id;
  ck->capacity = size;
  sendBufMgr->put(ck->buffer_id, ck);
}
