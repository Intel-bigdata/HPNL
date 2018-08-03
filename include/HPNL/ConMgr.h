#ifndef CONMGR_H
#define CONMGR_H

#include <rdma/fi_domain.h>
#include <rdma/fi_cm.h>

#include <thread>
#include <mutex>
#include <deque>

#include "HPNL/EventType.h"
#include "HPNL/ThreadWrapper.h"

class ConMgr;

struct ConEvent {
  ConEvent(fid_ep* ep_, EventType type_, void* addr_) : ep(ep_), type(type_), addr(addr_) {}
  ~ConEvent() {
    addr = NULL; 
  }
  fid_ep* ep;
  EventType type;
  void* addr;
};

class EventThread;

class ConMgr {
  public:
    ~ConMgr();
    void handle_event();
    void notify();
    void reap();

    void connect(fid_ep* ep, void* addr);
    void accept(fid_ep* ep);
    void shutdown(fid_ep* ep);

    void push_event(fid_ep*, EventType, void*);
    fid_ep* get_event();

  private:
    std::deque<ConEvent*> event_queue;
    std::mutex queue_mtx;
    std::condition_variable queue_cv;

    std::mutex event_mtx;
    std::condition_variable event_cv;
    bool ready = true;
    bool need_reap = false;

    EventThread *eventThread;
};

class EventThread : public ThreadWrapper {
  public:
    EventThread(ConMgr *conMgr_) : conMgr(conMgr_) {}
    virtual ~EventThread() {}
    virtual void entry() override {
      conMgr->handle_event(); 
    }
    virtual void abort() override {}
  private:
    ConMgr *conMgr;
};

#endif
