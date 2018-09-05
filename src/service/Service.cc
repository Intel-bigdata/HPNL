#include "HPNL/Service.h"

Service::Service(const char* ip_, const char* port_, bool is_server_) 
  : ip(ip_), port(port_), is_server(is_server_) {
  stack = new FIStack(ip, port, is_server ? FI_SOURCE : 0);
  recvCallback = NULL;
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

void Service::run(int con_num) {
  eq_demulti_plexer = new EQEventDemultiplexer();
  for (int i = 0; i < WORKERS; i++) {
    cq_demulti_plexer[i] = new CQEventDemultiplexer(stack, i); 
  }
  reactor = new Reactor(eq_demulti_plexer, cq_demulti_plexer);

  if (is_server) {
    con_num = 1; 
  }
  HandlePtr eqHandle[con_num];
  for (int i = 0; i< con_num; i++) {
    if (is_server) {
      eqHandle[i] = stack->bind();
      stack->listen();
    } else {
      assert(recvBufMgr);
      eqHandle[i] = stack->connect(recvBufMgr, sendBufMgr);
    }
    EventHandlerPtr handler(new EQHandler(stack, reactor, eqHandle[i]));
    acceptRequestCallback = new AcceptRequestCallback(this);
    handler->set_recv_callback(recvCallback);
    handler->set_send_callback(sendCallback);
    handler->set_accept_request_callback(acceptRequestCallback);
    handler->set_connected_callback(connectedCallback);
    handler->set_shutdown_callback(shutdownCallback);
    reactor->register_handler(handler);
  }
  
  eqThread = new EQThread(reactor);
  for (int i = 0; i < WORKERS; i++) {
    cqThread[i] = new CQThread(reactor, i); 
  }
  eqThread->start(); 
  for (int i = 0; i < WORKERS; i++) {
    cqThread[i]->start(); 
  }
}

void Service::shutdown() {
  for (int i = 0; i < WORKERS; i++) {
    cqThread[i]->stop(); 
    cqThread[i]->join();
  }
  eq_demulti_plexer->shutdown();
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

void Service::set_connected_callback(Callback *callback) {
  connectedCallback = callback;
}

void Service::set_shutdown_callback(Callback *callback) {
  shutdownCallback = callback;
}
