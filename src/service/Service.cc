#include "HPNL/Service.h"
#include "core/FiStack.h"
#include "core/FiConnection.h"
#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/CqDemultiplexer.h"
#include "demultiplexer/Proactor.h"
#include "demultiplexer/EqHandler.h"

Service::Service(int worker_num_, int buffer_num, bool is_server_) 
  : worker_num(worker_num_), is_server(is_server_) {
  recvCallback = NULL;
  sendCallback = NULL;
  readCallback = NULL;
  acceptRequestCallback = NULL;
  connectedCallback = NULL;
  shutdownCallback = NULL;

  stack = new FiStack(is_server ? FI_SOURCE : 0, worker_num, buffer_num, is_server);
}

Service::~Service() {
  // TODO: IOService deconstruction
  delete stack;
  delete eq_demulti_plexer;
  for (int i = 0; i < worker_num; i++) {
    delete cq_demulti_plexer[i]; 
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

int Service::init() {
  int res = 0;
  if ((res = stack->init())) {
    return res;
  }
  eq_demulti_plexer = new EqDemultiplexer(stack);
  eq_demulti_plexer->init();
  for (int i = 0; i < worker_num; i++) {
    cq_demulti_plexer[i] = new CqDemultiplexer(stack, i);
    cq_demulti_plexer[i]->init();
  }

  proactor = new Proactor(eq_demulti_plexer, cq_demulti_plexer, worker_num);
  return 0;
}

void Service::start() {
  eqThread = new EqThread(proactor);
  eqThread->start(); 
  for (int i = 0; i < worker_num; i++) {
    cqThread[i] = new CqThread(proactor, i);
    cqThread[i]->start(); 
  }
}

int Service::listen(const char* addr, const char* port) {
  fid_eq* eq = stack->bind(addr, port);
  stack->listen();

  std::shared_ptr<EqHandler> handler(new EqHandler(stack, proactor, eq));
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

int Service::connect(const char* addr, const char* port) {
  fid_eq *eq = stack->connect(addr, port, recvBufMgr, sendBufMgr);

  std::shared_ptr<EventHandler> handler(new EqHandler(stack, proactor, eq));
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

void Service::shutdown() {
  for (int i = 0; i < worker_num; i++) {
    cqThread[i]->stop(); 
    cqThread[i]->join();
  }
  eqThread->stop();
  eqThread->join();
}

void Service::shutdown(Connection *con) {
  proactor->remove_handler(((FiConnection*)con)->get_fid());
  ((FiConnection*)con)->shutdown();
  stack->reap(((FiConnection*)con)->get_fid());
  if (shutdownCallback) {
    shutdownCallback->operator()(NULL, NULL);
  }
  delete con;
}

void Service::wait() {
  eqThread->join();
}

void Service::set_recv_buf_mgr(BufMgr* bufMgr) {
  recvBufMgr = bufMgr;
}

void Service::set_send_buf_mgr(BufMgr* bufMgr) {
  sendBufMgr = bufMgr;
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
  return stack->reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void Service::unreg_rma_buffer(int buffer_id) {
  stack->unreg_rma_buffer(buffer_id);
}

Chunk* Service::get_rma_buffer(int buffer_id) {
  return stack->get_rma_chunk(buffer_id);
}

