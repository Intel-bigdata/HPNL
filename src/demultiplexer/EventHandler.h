#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "demultiplexer/EventType.h"
#include "demultiplexer/Handle.h"
#include "util/Ptr.h"
#include "util/Callback.h"

class EventHandler {
  public:
    virtual ~EventHandler() {}
    virtual int handle_event(EventType et, void *context) = 0;
    virtual HandlePtr get_handle(void) const = 0;

    virtual void set_connected_callback(Callback *callback) {}
    virtual void set_shutdown_callback(Callback *callback) {}
    virtual void set_read_callback(Callback *callback) {}
};

#endif
