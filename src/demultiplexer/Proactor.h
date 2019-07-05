#ifndef REACTOR_H
#define REACTOR_H

#include <map>
#include <list>

#include "HPNL/Common.h"
#include "demultiplexer/ThreadWrapper.h"

class EqDemultiplexer;
class CqDemultiplexer;
class RdmCqDemultiplexer;
class EventHandler;
class fid;

class Proactor {
  public:
    Proactor(EqDemultiplexer*, CqDemultiplexer**, int);
    Proactor(RdmCqDemultiplexer*);
    ~Proactor();
    int eq_service();
    int cq_service(int);
    int rdm_cq_service();
    int register_handler(std::shared_ptr<EventHandler>);
    int remove_handler(std::shared_ptr<EventHandler>);
    int remove_handler(fid*);
    int handle_events(int timeout = 0);
  private:
    std::mutex mtx;
    std::map<fid*, std::shared_ptr<EventHandler>> eventMap;
    std::map<fid*, std::shared_ptr<EventHandler>> curEventMap;
    EqDemultiplexer *eqDemultiplexer;
    CqDemultiplexer *cqDemultiplexer[MAX_WORKERS];
    RdmCqDemultiplexer *rdmCqDemultiplexer;
    int cq_worker_num;
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

class RdmCqThread : public ThreadWrapper {
  public:
    RdmCqThread(Proactor *proactor_) : proactor(proactor_) {}
    virtual ~RdmCqThread() {}
    virtual int entry() override {
      return proactor->rdm_cq_service();
    } 
    virtual void abort() override {}
  private:
    Proactor *proactor;
};

#endif
