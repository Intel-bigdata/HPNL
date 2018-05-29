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
      char buff[SIZE];
      memset(buff, '0', SIZE);
      con->write(buff, SIZE);
    }
};

class ReadCallback : public Callback {
  public:
    virtual ~ReadCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
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
      con->write(msg, SIZE);
    }
};

int main(int argc, char *argv[]) {
  LogPtr logger(new Log("tmp", "nanolog.client", nanolog::LogLevel::DEBUG));
  Stack *stack = new FIStack("127.0.0.1", "12345", 0); 
  EventDemultiplexer *plexer = new FIEventDemultiplexer((fid_domain*)stack->get_domain(), (fid_wait*)stack->get_wait_set(), logger);
  Reactor *reactor = new Reactor(plexer);

  HandlePtr handle = stack->connect();
  EventHandlerPtr handler(new CMHandler(stack, reactor, handle));
  ConnectedCallback *connectedCallback = new ConnectedCallback();
  ReadCallback *readCallback = new ReadCallback();
  handler->set_connected_callback(connectedCallback);
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
