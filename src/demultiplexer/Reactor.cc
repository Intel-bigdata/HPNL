#include "Reactor.h"

Reactor::Reactor(EventDemultiplexer *eventDemultiplexer_) : eventDemultiplexer(eventDemultiplexer_) {}

Reactor::~Reactor() {
  eventMap.erase(eventMap.begin(), eventMap.end()); 
}

void Reactor::operator()() {
  eventDemultiplexer->wait_event(eventMap);
}

int Reactor::register_handler(EventHandlerPtr eh) {
  HandlePtr handle = eh->get_handle();
  if (eventMap.find(handle) == eventMap.end()) {
    eventMap.insert(std::make_pair(handle, eh)); 
  }
  return eventDemultiplexer->register_event(handle);
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
  return eventDemultiplexer->remove_event(handle);
}
