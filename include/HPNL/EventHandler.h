#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "EventType.h"
#include "Handle.h"
#include "Ptr.h"
#include "Callback.h"

class EventHandler {
  public:
    virtual ~EventHandler() {}
    virtual int handle_event(EventType et, void *context) = 0;
    virtual HandlePtr get_handle(void) const = 0;

    virtual void set_conntected_callback(Callback *callback) {}
    virtual void set_shutdown_callback(Callback *callback) {}
    virtual void set_read_callback(Callback *callback) {}
};

#endif
