#include "HPNL/Reactor.h"

#include <iostream>

Reactor::Reactor(EventDemultiplexer *eqDemultiplexer_, CQEventDemultiplexer **cqDemultiplexer_) : eqDemultiplexer(eqDemultiplexer_) {
  for (int i = 0; i < WORKERS; i++) {
    cqDemultiplexer[i] = *(cqDemultiplexer_+i);
  }
}

Reactor::~Reactor() {
  eventMap.erase(eventMap.begin(), eventMap.end()); 
}

void Reactor::eq_service() {
  eqDemultiplexer->wait_event(eventMap);
}

void Reactor::cq_service(int num) {
  cqDemultiplexer[num]->wait_event();
}

int Reactor::register_handler(EventHandlerPtr eh) {
  HandlePtr handle = eh->get_handle();
  if (eventMap.find(handle) == eventMap.end()) {
    eventMap.insert(std::make_pair(handle, eh)); 
  }
  return eqDemultiplexer->register_event(handle);
}

int Reactor::remove_handler(EventHandlerPtr eh) {
  HandlePtr handle = eh->get_handle();
  return remove_handler(handle);
}

int Reactor::remove_handler(HandlePtr handle) {
  std::map<HandlePtr, EventHandlerPtr>::iterator iter = eventMap.find(handle);
  if (iter != eventMap.end())
    eventMap.erase(iter);
  else
    return -1;
  return eqDemultiplexer->remove_event(handle);
}

