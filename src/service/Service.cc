#include "service/Service.h"

Service::Service(const char* ip_, const char* port_, LogPtr logger_, bool is_server_) 
  : ip(ip_), port(port_), logger(logger_), is_server(is_server_) {
  stack = new FIStack(ip, port, is_server ? FI_SOURCE : 0);
  readCallback = NULL;
  sendCallback = NULL;
  acceptRequestCallback = NULL;
  connectedCallback = NULL;
  shutdownCallback = NULL;
}

Service::~Service() {
  // TODO: IOService deconstruction
  delete stack;
  delete eq_demulti_plexer;
  for (int i = 0; i < WORKERS; i++) {
    delete cq_demulti_plexer[i]; 
  }
  delete reactor;
  delete acceptRequestCallback;
  delete eqThread;
  for (int i = 0; i < WORKERS; i++) {
    delete cqThread[i];
  }
}

void Service::run() {
  HandlePtr eqHandle;
  if (is_server) {
    eqHandle = stack->bind();
    stack->listen();
  } else {
    assert(recvBufMgr);
    eqHandle = stack->connect(recvBufMgr, sendBufMgr);
  }

  eq_demulti_plexer = new EQEventDemultiplexer(logger);
  for (int i = 0; i < WORKERS; i++) {
    cq_demulti_plexer[i] = new CQEventDemultiplexer(stack, i); 
  }
  reactor = new Reactor(eq_demulti_plexer, cq_demulti_plexer);
  
  EventHandlerPtr handler(new EQHandler(stack, reactor, eqHandle));
  acceptRequestCallback = new AcceptRequestCallback(this);
  handler->set_read_callback(readCallback);
  handler->set_send_callback(sendCallback);
  handler->set_accept_request_callback(acceptRequestCallback);
  handler->set_connected_callback(connectedCallback);
  handler->set_shutdown_callback(shutdownCallback);
  reactor->register_handler(handler);

  eqThread = new EQThread(reactor);
  for (int i = 0; i < WORKERS; i++) {
    cqThread[i] = new CQThread(reactor, i); 
  }
  eqThread->start(); 
  for (int i = 0; i < WORKERS; i++) {
    cqThread[i]->start(true); 
  }
}

void Service::shutdown() {
  eqThread->stop();
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

void Service::set_read_callback(Callback *callback) {
  readCallback = callback;
}

void Service::set_send_callback(Callback *callback) {
  sendCallback = callback;
}

void Service::set_connected_callback(Callback *callback) {
  connectedCallback = callback;
}

void Service::set_shutdown_callback(Callback *callback) {
  shutdownCallback = callback;
}
