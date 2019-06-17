#include "external_service/ExternalRdmService.h"

#include "HPNL/Connection.h"
#include "core/RdmConnection.h"
#include "external_service/ExternalEqServiceBufMgr.h"
#include "external_demultiplexer/ExternalRdmCqDemultiplexer.h"
#include "core/RdmStack.h"

#include <iostream>

ExternalRdmService::ExternalRdmService(int buffer_num, bool is_server) {
  this->buffer_num = buffer_num;
  this->is_server = is_server;
  this->recvBufMgr = new ExternalEqServiceBufMgr();
  this->sendBufMgr = new ExternalEqServiceBufMgr();
}

ExternalRdmService::~ExternalRdmService() {
  delete this->stack;
  delete this->demulti_plexer;
  delete this->recvBufMgr;
  delete this->sendBufMgr;
}

int ExternalRdmService::init(const char* prov_name) {
  this->stack = new RdmStack(this->buffer_num, this->is_server, prov_name);
  this->stack->init();
  this->demulti_plexer = new ExternalRdmCqDemultiplexer(stack);
  this->demulti_plexer->init();
  return 0;
}

RdmConnection* ExternalRdmService::listen(const char* ip, const char* port) {
  return (RdmConnection*)stack->bind(ip, port, recvBufMgr, sendBufMgr);
}

RdmConnection* ExternalRdmService::get_con(const char* ip, const char* port) {
  RdmConnection *con = ((RdmStack*)stack)->get_con(ip, port, recvBufMgr, sendBufMgr);
  return (RdmConnection*)con;
}

int ExternalRdmService::wait_event(Chunk **ck, int *block_buffer_size) {
  return this->demulti_plexer->wait_event(ck, block_buffer_size);
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
