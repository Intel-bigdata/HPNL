#include "EventDemultiplexer.h"
#include "FIEventDemultiplexer.h"
#include "Stack.h"
#include "FIStack.h"
#include "Reactor.h"
#include "Ptr.h"
#include "EventHandler.h"
#include "CMHandler.h"
#include "Connection.h"
#include "ThreadWrapper.h"

#include <thread>
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
      //std::this_thread::sleep_for(std::chrono::seconds(2));
      char *msg = (char*)param_2;
      Connection *con = (Connection*)param_1; con->write(msg, 5);
    }
};

int main() {
  LogPtr logger(new Log("tmp", "nanolog.server", nanolog::LogLevel::WARN));
  Stack *stack = new FIStack("127.0.0.1", FI_SOURCE); 
  EventDemultiplexer *plexer = new FIEventDemultiplexer((fid_domain*)stack->get_domain(), logger);
  assert(stack->get_domain());
  Reactor *reactor = new Reactor(plexer);
  
  HandlePtr handle = stack->bind();
  assert(handle->get_event() == EQ_EVENT);
  EventHandlerPtr handler(new CMHandler(stack, reactor, handle));
  ReadCallback *readCallback = new ReadCallback();
  handler->set_conntected_callback(NULL);
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
