#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "core/ConMgr.h"
#include "HPNL/Callback.h"
#include "demultiplexer/EventType.h"
#include "demultiplexer/Handle.h"
#include "util/Ptr.h"

class EventHandler {
  public:
    virtual ~EventHandler() {}
    virtual int handle_event(EventType, void*) = 0;
    virtual HandlePtr get_handle(void) const = 0;

    virtual void set_accept_request_callback(Callback *callback) = 0;
    virtual void set_connected_callback(Callback *callback) = 0;
    virtual void set_shutdown_callback(Callback *callback) = 0;
    virtual void set_read_callback(Callback *callback) = 0;
    virtual void set_send_callback(Callback *callback) = 0;
    virtual Callback* get_read_callback() = 0;
};

#endif
