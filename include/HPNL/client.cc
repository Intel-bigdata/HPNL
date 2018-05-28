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
#include "ThreadWrapper.h"

int count = 0;
uint64_t start, end = 0;
uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

class ConnectThread : public ThreadWrapper {
  public:
    ConnectThread(Reactor *reactor_) : reactor(reactor_) {}
    virtual ~ConnectThread() {}

    virtual void entry() override {
      while (true) {
        (*reactor)();
      }
    }
    virtual void abort() override {}
  private:
    Reactor *reactor;
};

class ConnectedCallback : public Callback {
  public:
    virtual ~ConnectedCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      Connection *con = (Connection*)param_1; 
      char buff[4096] = "hello server";
      con->write(buff, 5);
    }
};

class ReadCallback : public Callback {
  public:
    virtual ~ReadCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      //std::this_thread::sleep_for(std::chrono::seconds(2));
      count++;
      Connection *con = (Connection*)param_1;
      if (count >= 1000000) {
        con->shutdown();
        end = timestamp_now();
        printf("finished, consumes %f s.\n", (end-start)/1000.0);
        return;
      }
      if (count == 1) {
        start = timestamp_now(); 
      }
      char *msg = (char*)param_2;
      con->write(msg, 5);
    }
};

int main() {
  LogPtr logger(new Log("tmp", "nanolog.client", nanolog::LogLevel::WARN));
  Stack *stack = new FIStack("127.0.0.1", 0); 
  EventDemultiplexer *plexer = new FIEventDemultiplexer((fid_domain*)stack->get_domain(), logger);
  Reactor *reactor = new Reactor(plexer);

  HandlePtr handle = stack->connect();
  EventHandlerPtr handler(new CMHandler(stack, reactor, handle));
  ConnectedCallback *connectedCallback = new ConnectedCallback();
  ReadCallback *readCallback = new ReadCallback();
  handler->set_conntected_callback(connectedCallback);
  handler->set_read_callback(readCallback);
  reactor->register_handler(handler);

  ConnectThread *thd = new ConnectThread(reactor);
  thd->start();
  thd->set_affinity(2);
  thd->join();

  delete stack;
  delete plexer;
  delete reactor;

  return 0;
}
