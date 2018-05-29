#ifndef CMHANDLER_H
#define CMHANDLER_H

#include "core/Stack.h"
#include "demultiplexer/Reactor.h"
#include "util/EventHandler.h"
#include "util/CQHandler.h"
#include "util/Callback.h"

class CMHandler : public EventHandler {
  public:
    CMHandler(Stack *stack_, Reactor *reactor_, HandlePtr handle_) : stack(stack_), reactor(reactor_), cmHandle(handle_) {}
    virtual ~CMHandler() {}
    virtual int handle_event(EventType et, void *context) override;
    virtual HandlePtr get_handle(void) const override;
    
    void set_read_callback(Callback *callback) override;
    void set_conntected_callback(Callback *callback) override;
    void set_shutdown_callback(Callback *callback) override;
  private:
    Stack *stack;
    Reactor *reactor;
    HandlePtr cmHandle;
    Callback *conntectedCallback;
    Callback *shutdownCallback;
    Callback *readCallback;
};

#endif
