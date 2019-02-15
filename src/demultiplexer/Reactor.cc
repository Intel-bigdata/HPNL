#include "HPNL/Reactor.h"

#include <iostream>

Reactor::Reactor(EventDemultiplexer *eqDemultiplexer_, CQEventDemultiplexer **cqDemultiplexer_, int worker_num) : eqDemultiplexer(eqDemultiplexer_) {
  for (int i = 0; i < worker_num; i++) {
    cqDemultiplexer[i] = *(cqDemultiplexer_+i);
  }
}

Reactor::~Reactor() {
  eventMap.erase(eventMap.begin(), eventMap.end()); 
}

int Reactor::eq_service() {
  return eqDemultiplexer->wait_event(eventMap);
}

int Reactor::cq_service(int index) {
  std::cout << "index " << index << std::endl;
  return cqDemultiplexer[index]->wait_event();
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

