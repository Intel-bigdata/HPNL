#ifndef CMHANDLER_H
#define CMHANDLER_H

#include "core/Stack.h"
#include "demultiplexer/Reactor.h"
#include "demultiplexer/EventHandler.h"
#include "demultiplexer/CQHandler.h"
#include "util/Callback.h"

class CMHandler : public EventHandler {
  public:
    CMHandler(Stack *stack_, Reactor *reactor_, HandlePtr handle_) : stack(stack_), reactor(reactor_), cmHandle(handle_), connectedCallback(NULL), shutdownCallback(NULL), readCallback(NULL) {}
    virtual ~CMHandler() {}
    virtual int handle_event(EventType et, void *context) override;
    virtual HandlePtr get_handle(void) const override;
    
    void set_read_callback(Callback *callback) override;
    void set_connected_callback(Callback *callback) override;
    void set_shutdown_callback(Callback *callback) override;
  private:
    Stack *stack;
    Reactor *reactor;
    HandlePtr cmHandle;
    Callback *connectedCallback;
    Callback *shutdownCallback;
    Callback *readCallback;
};

#endif
