#include <string.h>

#include "EventDemultiplexer.h"
#include "FIEventDemultiplexer.h"
#include "Stack.h"
#include "FIStack.h"
#include "Reactor.h"
#include "Ptr.h"
#include "EventHandler.h"
#include "CMHandler.h"
#include "Connection.h"

class ConnectedCallback : public Callback {
  public:
    virtual ~ConnectedCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      Connection *con = (Connection*)param_1; 
      char buff[4096];
      memset(buff, '0', 4096);
      con->write(buff, 4096);
      printf("client send buffer.\n");
    }
};

int main() {
  Stack *stack = new FIStack("127.0.0.1", 0); 
  EventDemultiplexer *plexer = new FIEventDemultiplexer((fid_domain*)stack->get_domain());
  Reactor *reactor = new Reactor(plexer);

  HandlePtr handle = stack->connect();
  EventHandlerPtr handler(new CMHandler(stack, reactor, handle));
  handler->set_conntected_callback(NULL);
  ConnectedCallback *connectedCallback = new ConnectedCallback();
  handler->set_conntected_callback(connectedCallback);
  reactor->register_handler(handler);

  while(true) {
    (*reactor)(); 
  }

  delete stack;
  delete plexer;
  delete reactor;

  return 0;
}
