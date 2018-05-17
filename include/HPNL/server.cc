#include "EventDemultiplexer.h"
#include "FIEventDemultiplexer.h"
#include "Stack.h"
#include "FIStack.h"
#include "Reactor.h"
#include "Ptr.h"
#include "EventHandler.h"
#include "CMHandler.h"
#include "Connection.h"

class ReadCallback : public Callback {
  public:
    virtual ~ReadCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      printf("server read buffer.\n");
    }
};

int main() {
  Stack *stack = new FIStack("127.0.0.1", FI_SOURCE); 
  EventDemultiplexer *plexer = new FIEventDemultiplexer((fid_domain*)stack->get_domain());
  assert(stack->get_domain());
  Reactor *reactor = new Reactor(plexer);
  
  HandlePtr handle = stack->bind();
  assert(handle->get_event() == EQ_EVENT);
  EventHandlerPtr handler(new CMHandler(stack, reactor, handle));
  handler->set_conntected_callback(NULL);
  ReadCallback *readCallback = new ReadCallback();
  handler->set_read_callback(readCallback);
  reactor->register_handler(handler);
  stack->listen();

  while(true) {
    (*reactor)(); 
  }

  delete stack;
  delete plexer;
  delete reactor;
  delete readCallback;

  return 0;
}
