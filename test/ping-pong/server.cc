#include "core/Stack.h"
#include "core/FIStack.h"
#include "core/Connection.h"
#include "demultiplexer/EventDemultiplexer.h"
#include "demultiplexer/FIEventDemultiplexer.h"
#include "demultiplexer/Reactor.h"
#include "demultiplexer/EventHandler.h"
#include "demultiplexer/CMHandler.h"
#include "util/Ptr.h"
#include "util/ThreadWrapper.h"

#define SIZE 4096

class AcceptThread : public ThreadWrapper {
  public:
    AcceptThread(Reactor *reactor_) : reactor(reactor_) {}
    virtual ~AcceptThread() {}

    virtual void entry() override {
      while (true) {
        (*reactor)();
      }
    }
    virtual void abort() override {}
  private:
    Reactor *reactor;
};

class ReadCallback : public Callback {
  public:
    virtual ~ReadCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      char *msg = (char*)param_2;
      Connection *con = (Connection*)param_1; con->write(msg, SIZE);
    }
};

int main(int argc, char *argv[]) {
  LogPtr logger(new Log("tmp", "nanolog.server", nanolog::LogLevel::DEBUG));
  Stack *stack = new FIStack("127.0.0.1", "12345", FI_SOURCE); 
  EventDemultiplexer *plexer = new FIEventDemultiplexer((fid_domain*)stack->get_domain(), (fid_wait*)stack->get_wait_set(), logger);
  assert(stack->get_domain());
  Reactor *reactor = new Reactor(plexer);
  
  HandlePtr handle = stack->bind();
  assert(handle->get_event() == EQ_EVENT);
  EventHandlerPtr handler(new CMHandler(stack, reactor, handle));
  ReadCallback *readCallback = new ReadCallback();
  handler->set_connected_callback(NULL);
  handler->set_read_callback(readCallback);
  reactor->register_handler(handler);
  stack->listen();
  AcceptThread *thd = new AcceptThread(reactor);
  thd->start();
  thd->set_affinity(3);
  thd->join();

  delete stack;
  delete plexer;
  delete reactor;
  delete readCallback;

  return 0;
}
