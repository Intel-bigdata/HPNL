#include "Service.h"
#include "core/Stack.h"
#include "core/MsgStack.h"
#include "core/MsgConnection.h"
#include "core/RdmConnection.h"
#include "core/RdmStack.h"
#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/CqDemultiplexer.h"
#include "demultiplexer/RdmCqDemultiplexer.h"
#include "demultiplexer/Proactor.h"
#include "demultiplexer/EqHandler.h"

Service::Service(int worker_num_, int buffer_num_, bool is_server_) 
  : worker_num(worker_num_), buffer_num(buffer_num_), is_server(is_server_), msg(true) {
  stack = nullptr;
  proactor = nullptr;
  eq_demultiplexer = nullptr;
  rdm_cq_demultiplexer = nullptr;
  eqThread = nullptr;
  rdmCqThread = nullptr;
  recvCallback = nullptr;
  sendCallback = nullptr;
  readCallback = nullptr;
  acceptRequestCallback = nullptr;
  connectedCallback = nullptr;
  shutdownCallback = nullptr;
}

Service::~Service() {
  // TODO: IOService deconstruction
  delete stack;
  delete eq_demultiplexer;
  for (int i = 0; i < worker_num; i++) {
    delete cq_demultiplexer[i];
    if (!is_server) break;
  }
  delete proactor;
  delete acceptRequestCallback;
  delete eqThread;
  for (int i = 0; i < worker_num; i++) {
    delete cqThread[i];
    if (!is_server) break;
  }
}

int Service::init(bool msg_) {
  int res = 0;
  msg = msg_;
  if (msg) {
    stack = new MsgStack(worker_num, buffer_num, is_server);
    if ((res = stack->init())) {
      return res;
    }
    eq_demultiplexer = new EqDemultiplexer((MsgStack*)stack);
    eq_demultiplexer->init();
    for (int i = 0; i < worker_num; i++) {
      cq_demultiplexer[i] = new CqDemultiplexer((MsgStack*)stack, i);
      cq_demultiplexer[i]->init();
    }
    proactor = new Proactor(eq_demultiplexer, cq_demultiplexer, worker_num);
  } else {
    stack = new RdmStack(buffer_num, is_server);
    if ((res = stack->init())) {
      return res;
    }
    rdm_cq_demultiplexer = new RdmCqDemultiplexer((RdmStack*)stack);
    rdm_cq_demultiplexer->init();
    proactor = new Proactor(rdm_cq_demultiplexer);
  }
  
  return 0;
}

void Service::start() {
  if (msg) {
    eqThread = new EqThread(proactor);
    eqThread->start();
    for (int i = 0; i < worker_num; i++) {
      cqThread[i] = new CqThread(proactor, i);
      cqThread[i]->start(); 
    }
  } else {
    rdmCqThread = new RdmCqThread(proactor);
    rdmCqThread->start(); 
  }
}

int Service::listen(const char* addr, const char* port) {
  if (msg) {
    auto* eq = (fid_eq*)stack->bind(addr, port, bufMgr);
    ((MsgStack*)stack)->listen();
    std::shared_ptr<EqHandler> handler(new EqHandler((MsgStack*)stack, proactor, eq));
    acceptRequestCallback = new AcceptRequestCallback(this);
    handler->set_recv_callback(recvCallback);
    handler->set_send_callback(sendCallback);
    handler->set_read_callback(readCallback);
    handler->set_accept_request_callback(acceptRequestCallback);
    handler->set_connected_callback(connectedCallback);
    handler->set_shutdown_callback(shutdownCallback);
    proactor->register_handler(handler);
  } else {
    auto* con = (RdmConnection*)stack->bind(addr, port, bufMgr);
    con->set_recv_callback(recvCallback);
    con->set_send_callback(sendCallback);
  }

  return 0;
}

int Service::connect(const char* addr, const char* port) {
  fid_eq *eq = ((MsgStack*)stack)->connect(addr, port, bufMgr);

  std::shared_ptr<EventHandler> handler(new EqHandler((MsgStack*)stack, proactor, eq));
  acceptRequestCallback = new AcceptRequestCallback(this);
  handler->set_recv_callback(recvCallback);
  handler->set_send_callback(sendCallback);
  handler->set_read_callback(readCallback);
  handler->set_accept_request_callback(acceptRequestCallback);
  handler->set_connected_callback(connectedCallback);
  handler->set_shutdown_callback(shutdownCallback);
  proactor->register_handler(handler);

  return 0;
}

Connection* Service::get_con(const char* addr, const char* port) {
  RdmConnection *con = ((RdmStack*)stack)->get_con(addr, port, bufMgr);
  if (!con) {
    return nullptr;
  }
  con->set_recv_callback(recvCallback);
  con->set_send_callback(sendCallback);
  return (Connection*)con;
}

void Service::shutdown() {
  for (int i = 0; i < worker_num; i++) {
    if (cqThread[i]) {
      cqThread[i]->stop(); 
      cqThread[i]->join();
    }
  }
  if (eqThread) {
    eqThread->stop();
    eqThread->join();
  }
  if (rdmCqThread) {
    rdmCqThread->stop(); 
    rdmCqThread->join(); 
  }
}

void Service::shutdown(Connection *con) {
  proactor->remove_handler(((MsgConnection*)con)->get_fid());
  ((MsgConnection*)con)->shutdown();
  ((MsgStack*)stack)->reap(((MsgConnection*)con)->get_fid());
  if (shutdownCallback) {
    shutdownCallback->operator()(nullptr, nullptr);
  }
  delete con;
}

void Service::wait() {
  if (eqThread) {
    eqThread->join(); 
  }
  if (rdmCqThread) {
    rdmCqThread->join(); 
  }
}

void Service::set_buf_mgr(BufMgr* bufMgr_) {
  this->bufMgr = bufMgr_;
}

void Service::set_recv_callback(Callback *callback) {
  recvCallback = callback;
}

void Service::set_send_callback(Callback *callback) {
  sendCallback = callback;
}

void Service::set_read_callback(Callback *callback) {
  readCallback = callback;
}

void Service::set_connected_callback(Callback *callback) {
  connectedCallback = callback;
}

void Service::set_shutdown_callback(Callback *callback) {
  shutdownCallback = callback;
}

uint64_t Service::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  return ((MsgStack*)stack)->reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void Service::unreg_rma_buffer(int buffer_id) {
  ((MsgStack*)stack)->unreg_rma_buffer(buffer_id);
}

Chunk* Service::get_rma_buffer(int buffer_id) {
  return ((MsgStack*)stack)->get_rma_chunk(buffer_id);
}

