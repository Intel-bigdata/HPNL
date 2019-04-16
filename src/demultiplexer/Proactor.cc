#include "demultiplexer/Proactor.h"
#include "demultiplexer/EventType.h"
#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/CqDemultiplexer.h"
#include "demultiplexer/EventHandler.h"
#include "demultiplexer/Handle.h"

#include <iostream>

Proactor::Proactor(EqDemultiplexer *eqDemultiplexer_, CqDemultiplexer **cqDemultiplexer_, int worker_num) : eqDemultiplexer(eqDemultiplexer_) {
  for (int i = 0; i < worker_num; i++) {
    cqDemultiplexer[i] = *(cqDemultiplexer_+i);
  }
}

Proactor::~Proactor() {
  eventMap.erase(eventMap.begin(), eventMap.end()); 
}

int Proactor::eq_service() {
  return eqDemultiplexer->wait_event(eventMap);
}

int Proactor::cq_service(int index) {
  return cqDemultiplexer[index]->wait_event();
}

int Proactor::register_handler(std::shared_ptr<EventHandler> eh) {
  std::shared_ptr<Handle> handle = eh->get_handle();
  if (eventMap.find(handle) == eventMap.end()) {
    eventMap.insert(std::make_pair(handle, eh)); 
  }
  return eqDemultiplexer->register_event(handle);
}

int Proactor::remove_handler(std::shared_ptr<EventHandler> eh) {
  std::shared_ptr<Handle> handle = eh->get_handle();
  return remove_handler(handle);
}

int Proactor::remove_handler(std::shared_ptr<Handle> handle) {
  std::map<std::shared_ptr<Handle>, std::shared_ptr<EventHandler>>::iterator iter = eventMap.find(handle);
  if (iter != eventMap.end())
    eventMap.erase(iter);
  else
    return -1;
  return eqDemultiplexer->remove_event(handle);
}

