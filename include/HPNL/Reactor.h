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
    Reactor(EventDemultiplexer*, CQEventDemultiplexer**, int);
    ~Reactor();
    int eq_service();
    int cq_service(int);
    int register_handler(EventHandlerPtr);
    int remove_handler(EventHandlerPtr);
    int remove_handler(HandlePtr);
    int handle_events(int timeout = 0);
  private:
    std::map<HandlePtr, EventHandlerPtr> eventMap;
    EventDemultiplexer *eqDemultiplexer;
    CQEventDemultiplexer *cqDemultiplexer[MAX_WORKERS];
};

class EQThread : public ThreadWrapper {
  public:
    EQThread(Reactor *reactor_) : reactor(reactor_) {}
    virtual ~EQThread() {}
    virtual int entry() override {
      return reactor->eq_service();
    }
    virtual void abort() override {}
  private:
    Reactor *reactor;
};

class CQThread : public ThreadWrapper {
  public:
    CQThread(Reactor *reactor_, int index_) : reactor(reactor_), index(index_) {}
    virtual ~CQThread() {}
    virtual int entry() override {
      return reactor->cq_service(index);
    } 
    virtual void abort() override {}
  private:
    Reactor *reactor;
    int index;
};

#endif
