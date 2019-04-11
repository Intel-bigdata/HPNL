#include "HPNL/ExternalEqService.h"

ExternalEqService::ExternalEqService(int worker_num_, int buffer_num_, bool is_server_) : worker_num(worker_num_), buffer_num(buffer_num_), is_server(is_server_) {
  recvBufMgr = new ExternalEqServiceBufMgr();
  sendBufMgr = new ExternalEqServiceBufMgr();
}

ExternalEqService::~ExternalEqService() {
  if (stack) {
    delete stack;
    stack = nullptr;
  }
  if (eq_demulti_plexer) {
    delete eq_demulti_plexer;
    eq_demulti_plexer = nullptr;
  }
  if (recvBufMgr) {
    delete recvBufMgr;
    recvBufMgr = nullptr;
  }
  if (sendBufMgr) {
    delete sendBufMgr;
    sendBufMgr = nullptr;
  }
}

int ExternalEqService::init() {
  stack = new FIStack(is_server ? FI_SOURCE : 0, worker_num, buffer_num, is_server);
  if (stack->init() == -1)
    goto free_stack;

  eq_demulti_plexer = new EQExternalDemultiplexer(stack);
  if (eq_demulti_plexer->init() == -1)
    goto free_eq_demulti_plexer;

  return 0;

free_eq_demulti_plexer:
  if (eq_demulti_plexer) {
    delete eq_demulti_plexer;
    eq_demulti_plexer = nullptr;
  }
free_stack:
  if (stack) {
    delete stack;
    stack = nullptr;
  }

  return -1;
}

fid_eq* ExternalEqService::accept(fi_info* info) {
  if (sendBufMgr->free_size() < buffer_num || recvBufMgr->free_size() < buffer_num*2)
    return NULL;
  HandlePtr eqHandle;
  eqHandle = stack->accept(info, recvBufMgr, sendBufMgr);
  if (!eqHandle)
    return NULL;
  return (fid_eq*)eqHandle->get_ctx();
}

fid_eq* ExternalEqService::connect(const char* ip, const char* port) {
  HandlePtr eqHandle;
  if (is_server) {
    eqHandle = stack->bind(ip, port);
    if (!eqHandle)
      return NULL;
    if (stack->listen()) {
      return NULL; 
    }
  } else {
    if (sendBufMgr->free_size() < buffer_num || recvBufMgr->free_size() < buffer_num*2)
      return NULL;
    eqHandle = stack->connect(ip, port, recvBufMgr, sendBufMgr);
    if (!eqHandle)
      return NULL;
  }
  return (fid_eq*)eqHandle->get_ctx();
}

uint64_t ExternalEqService::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  return stack->reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void ExternalEqService::unreg_rma_buffer(int buffer_id) {
  stack->unreg_rma_buffer(buffer_id);
}

Chunk* ExternalEqService::get_rma_buffer(int buffer_id) {
  return stack->get_rma_chunk(buffer_id);
}

void ExternalEqService::set_recv_buffer(char* buffer, uint64_t size, int buffer_id) {
  Chunk *ck = new Chunk();
  ck->buffer = buffer;
  ck->buffer_id = buffer_id;
  ck->capacity = size;
  recvBufMgr->add(ck->buffer_id, ck);
}

void ExternalEqService::set_send_buffer(char* buffer, uint64_t size, int buffer_id) {
  Chunk *ck = new Chunk();
  ck->buffer = buffer;
  ck->buffer_id = buffer_id;
  ck->capacity = size;
  sendBufMgr->add(ck->buffer_id, ck);
}

int ExternalEqService::wait_eq_event(fi_info** info, fid_eq** eq, FIConnection** con) {
  int ret = eq_demulti_plexer->wait_event(info, eq, con);
  return ret;
}

Connection* ExternalEqService::get_connection(fid_eq* eq) {
  FIConnection *con = stack->get_connection(&eq->fid);
  return con;
}

void ExternalEqService::reap(fid *con_id) {
  stack->reap(con_id);
}

FIStack* ExternalEqService::get_stack() {
  return stack;
}

Chunk* ExternalEqService::get_chunk(int id, int type) {
  Chunk *ck = NULL;
  if (type == RECV_CHUNK)
    ck = recvBufMgr->index(id);
  else
    ck = sendBufMgr->index(id);
  return ck;
}

int ExternalEqService::get_worker_num() {
  return worker_num;
}

int ExternalEqService::add_eq_event(fid_eq *eq) {
  eq_demulti_plexer->add_event(eq);
  return 0;
}

int ExternalEqService::delete_eq_event(fid_eq *eq) {
  eq_demulti_plexer->delete_event(eq);
  return 0;
}
