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
#include "HPNL/CqDemultiplexer.h"

class Proactor {
  public:
    Proactor(EventDemultiplexer*, CqDemultiplexer**, int);
    ~Proactor();
    int eq_service();
    int cq_service(int);
    int register_handler(EventHandlerPtr);
    int remove_handler(EventHandlerPtr);
    int remove_handler(HandlePtr);
    int handle_events(int timeout = 0);
  private:
    std::map<HandlePtr, EventHandlerPtr> eventMap;
    EventDemultiplexer *eqDemultiplexer;
    CqDemultiplexer *cqDemultiplexer[MAX_WORKERS];
};

class EQThread : public ThreadWrapper {
  public:
    EQThread(Proactor *proactor_) : proactor(proactor_) {}
    virtual ~EQThread() {}
    virtual int entry() override {
      return proactor->eq_service();
    }
    virtual void abort() override {}
  private:
    Proactor *proactor;
};

class CQThread : public ThreadWrapper {
  public:
    CQThread(Proactor *proactor_, int index_) : proactor(proactor_), index(index_) {}
    virtual ~CQThread() {}
    virtual int entry() override {
      return proactor->cq_service(index);
    } 
    virtual void abort() override {}
  private:
    Proactor *proactor;
    int index;
};

#endif
