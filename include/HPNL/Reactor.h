#ifndef REACTOR_H
#define REACTOR_H

#include <map>
#include <list>

#include "HPNL/Common.h"
#include "HPNL/EventType.h"
#include "HPNL/EventDemultiplexer.h"
#include "HPNL/EventHandler.h"
#include "HPNL/Ptr.h"
#include "HPNL/ThreadWrapper.h"
#include "HPNL/CQEventDemultiplexer.h"

class Reactor {
  public:
    Reactor(EventDemultiplexer* eqDemultiplexer_, CQEventDemultiplexer** cqDemultiplexer_);
    ~Reactor();
    void eq_service();
    void cq_service(int num);
    int register_handler(EventHandlerPtr eh);
    int remove_handler(EventHandlerPtr eh);
    int remove_handler(HandlePtr handle);
    int handle_events(int timeout = 0);
  private:
    std::map<HandlePtr, EventHandlerPtr> eventMap;
    EventDemultiplexer *eqDemultiplexer;
    CQEventDemultiplexer *cqDemultiplexer[WORKERS];
};

class EQThread : public ThreadWrapper {
  public:
    EQThread(Reactor *reactor_) : reactor(reactor_) {}
    virtual ~EQThread() {}
    virtual void entry() override {
      reactor->eq_service();
    }
    virtual void abort() override {}
  private:
    Reactor *reactor;
};

class CQThread : public ThreadWrapper {
  public:
    CQThread(Reactor *reactor_, int num) : reactor(reactor_), worker_num(num) {}
    virtual ~CQThread() {}
    virtual void entry() override {
      reactor->cq_service(worker_num);
    } 
    virtual void abort() override {}
  private:
    Reactor *reactor;
    int worker_num;
};

#endif
