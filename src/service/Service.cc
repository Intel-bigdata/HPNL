#include "HPNL/Service.h"

Service::Service(bool is_server_) 
  : is_server(is_server_) {
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
  for (int i = 0; i < worker_num; i++) {
    delete cq_demulti_plexer[i]; 
    if (!is_server) break;
  }
  delete reactor;
  delete acceptRequestCallback;
  delete eqThread;
  for (int i = 0; i < worker_num; i++) {
    delete cqThread[i];
    if (!is_server) break;
  }
}

void Service::run(const char* ip_, const char* port_, int worker_num, int buffer_num) {
  if (is_server) {
    stack = new FIStack(FI_SOURCE, worker_num, buffer_num);
  } else {
    stack = new FIStack(0, 1, buffer_num);
  }
  stack->init();
  eq_demulti_plexer = new EQEventDemultiplexer();
  for (int i = 0; i < worker_num; i++) {
    cq_demulti_plexer[i] = new CQEventDemultiplexer(stack, i);
    if (!is_server) break;
  }

  assert(recvBufMgr);
  if (is_server) {
    reactor = new Reactor(eq_demulti_plexer, cq_demulti_plexer, worker_num);
    HandlePtr eqHandle;
    eqHandle = stack->bind(ip_, port_);
    stack->listen();

    EventHandlerPtr handler(new EQHandler(stack, reactor, eqHandle));
    acceptRequestCallback = new AcceptRequestCallback(this);
    handler->set_recv_callback(recvCallback);
    handler->set_send_callback(sendCallback);
    handler->set_accept_request_callback(acceptRequestCallback);
    handler->set_connected_callback(connectedCallback);
    handler->set_shutdown_callback(shutdownCallback);
    reactor->register_handler(handler);
  } else {
    reactor = new Reactor(eq_demulti_plexer, cq_demulti_plexer, 1);
    HandlePtr eqHandle[worker_num];
    for (int i = 0; i< worker_num; i++) {
      eqHandle[i] = stack->connect(ip_, port_, recvBufMgr, sendBufMgr);

      EventHandlerPtr handler(new EQHandler(stack, reactor, eqHandle[i]));
      acceptRequestCallback = new AcceptRequestCallback(this);
      handler->set_recv_callback(recvCallback);
      handler->set_send_callback(sendCallback);
      handler->set_accept_request_callback(acceptRequestCallback);
      handler->set_connected_callback(connectedCallback);
      handler->set_shutdown_callback(shutdownCallback);
      reactor->register_handler(handler);
    }
  }
    
  eqThread = new EQThread(reactor);
  for (int i = 0; i < worker_num; i++) {
    cqThread[i] = new CQThread(reactor, i);
    if (!is_server) break;
  }
  eqThread->start(); 
  for (int i = 0; i < worker_num; i++) {
    cqThread[i]->start(); 
    if (!is_server) break;
  }
}

void Service::shutdown() {
  for (int i = 0; i < worker_num; i++) {
    cqThread[i]->stop(); 
    cqThread[i]->join();
    if (!is_server) break;
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
