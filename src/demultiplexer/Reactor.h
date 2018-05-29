#ifndef REACTOR_H
#define REACTOR_H

#include <map>
#include <list>

#include "demultiplexer/EventDemultiplexer.h"
#include "demultiplexer/EventType.h"
#include "demultiplexer/EventHandler.h"
#include "util/Ptr.h"

class Reactor {
  public:
    Reactor(EventDemultiplexer* eventDemultiplexer_);
    ~Reactor();
    void operator()();
    int register_handler(EventHandlerPtr eh);
    int remove_handler(EventHandlerPtr eh);
    int remove_handler(HandlePtr handle);
    int handle_events(int timeout = 0);
  private:
    std::map<HandlePtr, EventHandlerPtr> eventMap;
    EventDemultiplexer *eventDemultiplexer;

};

#endif
