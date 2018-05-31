#ifndef EQHANDLER_H
#define EQHANDLER_H

#include <rdma/fi_cm.h>

#include "core/FIStack.h"
#include "core/FIConnection.h"
#include "demultiplexer/Reactor.h"
#include "demultiplexer/EventHandler.h"
#include "util/Callback.h"

class EQHandler : public EventHandler {
  public:
    EQHandler(FIStack *stack_, Reactor *reactor_, HandlePtr handle_) : stack(stack_), reactor(reactor_), eqHandle(handle_), connectedCallback(NULL), shutdownCallback(NULL) {}
    virtual ~EQHandler() {}
    virtual int handle_event(EventType et, void *context) override;
    virtual HandlePtr get_handle(void) const override;
    
    virtual void set_connected_callback(Callback *callback) override;
    virtual void set_shutdown_callback(Callback *callback) override;
    virtual void set_read_callback(Callback *callback) override;
    virtual Callback* get_read_callback() override;
    
  private:
    FIStack *stack;
    Reactor *reactor;
    HandlePtr eqHandle;
    Callback *readCallback;
    Callback *connectedCallback;
    Callback *shutdownCallback;
};

#endif
