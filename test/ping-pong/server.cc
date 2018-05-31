#include "core/Stack.h"
#include "core/FIStack.h"
#include "core/Connection.h"
#include "demultiplexer/EventDemultiplexer.h"
#include "demultiplexer/EQEventDemultiplexer.h"
#include "demultiplexer/Reactor.h"
#include "demultiplexer/EventHandler.h"
#include "demultiplexer/EQHandler.h"
#include "demultiplexer/CQEventDemultiplexer.h"
#include "util/Ptr.h"
#include "util/ThreadWrapper.h"

#define SIZE 3

class ReadCallback : public Callback {
  public:
    virtual ~ReadCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      char *msg = (char*)param_2;
      Connection *con = (Connection*)param_1;
      con->write(msg, SIZE);
    }
};

int main(int argc, char *argv[]) {
  LogPtr logger(new Log("/tmp/", "nanolog.server", nanolog::LogLevel::DEBUG));
  FIStack *stack = new FIStack("172.168.2.106", "12345", FI_SOURCE); 
  HandlePtr eqHandle = stack->bind();
  fid_cq** cqs = stack->get_cqs();
  CQEventDemultiplexer *epolls[WORKERS];
  for (int i = 0; i < WORKERS; i++) {
    epolls[i]= new CQEventDemultiplexer(stack->get_fabric(), cqs[i]);
  }
  EventDemultiplexer *eq_plexer = new EQEventDemultiplexer(logger);
  Reactor *reactor = new Reactor(eq_plexer, epolls);
  
  stack->listen();
  EventHandlerPtr handler(new EQHandler(stack, reactor, eqHandle));
  ReadCallback *readCallback = new ReadCallback();
  handler->set_connected_callback(NULL);
  handler->set_read_callback(readCallback);
  reactor->register_handler(handler);

  EQThread *eqThread = new EQThread(reactor);
  eqThread->start(true);
  CQThread *cqThread[WORKERS];
  for (int i = 0; i < WORKERS; i++) {
    cqThread[i] = new CQThread(reactor, i); 
    cqThread[i]->start(true);
  }

  eqThread->join();

  delete stack;
  delete reactor;
  delete readCallback;

  return 0;
}
