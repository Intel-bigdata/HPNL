#include "core/Stack.h"
#include "core/FIStack.h"
#include "core/Connection.h"
#include "demultiplexer/EventDemultiplexer.h"
#include "demultiplexer/EQEventDemultiplexer.h"
#include "demultiplexer/Reactor.h"
#include "demultiplexer/EventHandler.h"
#include "demultiplexer/EQHandler.h"
#include "util/Ptr.h"
#include "util/ThreadWrapper.h"

#define SIZE 3

int count = 0;
uint64_t start, end = 0;

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

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
  FIStack *stack = new FIStack("172.168.2.106", "12345", 0); 
  EventDemultiplexer *plexer = new EQEventDemultiplexer(logger);
  fid_cq** cqs = stack->get_cqs();
  CQEventDemultiplexer *epolls[WORKERS];
  for (int i = 0; i < WORKERS; i++) {
    epolls[i]= new CQEventDemultiplexer(stack->get_fabric(), cqs[i]);
  }
  Reactor *reactor = new Reactor(plexer, epolls);

  HandlePtr handle = stack->connect();
  EventHandlerPtr handler(new EQHandler(stack, reactor, handle));
  ConnectedCallback *connectedCallback = new ConnectedCallback();
  ReadCallback *readCallback = new ReadCallback();
  handler->set_connected_callback(connectedCallback);
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
  delete plexer;
  delete reactor;

  return 0;
}
