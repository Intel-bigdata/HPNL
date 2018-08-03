#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "HPNL/Callback.h"
#include "HPNL/EventType.h"
#include "HPNL/ConMgr.h"
#include "HPNL/Handle.h"
#include "HPNL/Ptr.h"

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
