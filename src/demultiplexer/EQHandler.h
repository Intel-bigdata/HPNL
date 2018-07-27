#ifndef EQHANDLER_H
#define EQHANDLER_H

#include <rdma/fi_cm.h>

#include "HPNL/Callback.h"
#include "core/FIStack.h"
#include "core/FIConnection.h"
#include "demultiplexer/Reactor.h"
#include "demultiplexer/EventHandler.h"

class EQHandler : public EventHandler {
  public:
    EQHandler(FIStack *stack_, Reactor *reactor_, HandlePtr handle_) : stack(stack_), reactor(reactor_), eqHandle(handle_), readCallback(NULL), sendCallback(NULL), acceptRequestCallback(NULL), connectedCallback(NULL), shutdownCallback(NULL) {}
    virtual ~EQHandler() {}
    virtual int handle_event(EventType, void*) override;
    virtual HandlePtr get_handle(void) const override;

    virtual void set_accept_request_callback(Callback*) override;
    virtual void set_connected_callback(Callback*) override;
    virtual void set_shutdown_callback(Callback*) override;
    virtual void set_send_callback(Callback*) override;
    virtual void set_read_callback(Callback*) override;
    virtual Callback* get_read_callback() override;
    
  private:
    FIStack *stack;
    Reactor *reactor;
    HandlePtr eqHandle;
    Callback *readCallback;
    Callback *sendCallback;
    Callback *acceptRequestCallback;
    Callback *connectedCallback;
    Callback *shutdownCallback;
};

#endif
