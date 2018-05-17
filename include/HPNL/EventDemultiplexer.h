#ifndef EVENTDEMULTIPLEXER_H
#define EVENTDEMULTIPLEXER_H

#include <map>

#include "Handle.h"
#include "EventHandler.h"

class EventDemultiplexer {
  public:
    virtual ~EventDemultiplexer() {}
    virtual int wait_event(std::map<HandlePtr, EventHandlerPtr> &eventMap) = 0;
    virtual int register_event(HandlePtr) = 0;
    virtual int remove_event(HandlePtr) = 0;
};

#endif
