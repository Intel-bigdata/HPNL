#ifndef REACTOR_H
#define REACTOR_H

#include <map>
#include <list>

#include "HPNL/Common.h"
#include "demultiplexer/ThreadWrapper.h"

class EqDemultiplexer;
class CqDemultiplexer;
class EventHandler;
class fid;

class Proactor {
  public:
    Proactor(EqDemultiplexer*, CqDemultiplexer**, int);
    ~Proactor();
    int eq_service();
    int cq_service(int);
    int register_handler(std::shared_ptr<EventHandler>);
    int remove_handler(std::shared_ptr<EventHandler>);
    int remove_handler(fid*);
    int handle_events(int timeout = 0);
  private:
    std::map<fid*, std::shared_ptr<EventHandler>> eventMap;
    EqDemultiplexer *eqDemultiplexer;
    CqDemultiplexer *cqDemultiplexer[MAX_WORKERS];
};

class EqThread : public ThreadWrapper {
  public:
    EqThread(Proactor *proactor_) : proactor(proactor_) {}
    virtual ~EqThread() {}
    virtual int entry() override {
      return proactor->eq_service();
    }
    virtual void abort() override {}
  private:
    Proactor *proactor;
};

class CqThread : public ThreadWrapper {
  public:
    CqThread(Proactor *proactor_, int index_) : proactor(proactor_), index(index_) {}
    virtual ~CqThread() {}
    virtual int entry() override {
      return proactor->cq_service(index);
    } 
    virtual void abort() override {}
  private:
    Proactor *proactor;
    int index;
};

#endif
