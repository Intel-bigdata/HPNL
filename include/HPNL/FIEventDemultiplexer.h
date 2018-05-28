#ifndef RDMAEVENTDEMULTIPLEXER_H
#define RDMAEVENTDEMULTIPLEXER_H

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>

#include <thread>

#include "EventDemultiplexer.h"
#include "Handle.h"
#include "Ptr.h"

#define MAX_POLL_CNT 8

class FIEventDemultiplexer : public EventDemultiplexer {
  public:
    FIEventDemultiplexer(fid_domain *domain, LogPtr logger_);
    virtual ~FIEventDemultiplexer() override;
    virtual int wait_event(std::map<HandlePtr, EventHandlerPtr> &eventMap) override;
    virtual int register_event(HandlePtr handle) override;
    virtual int remove_event(HandlePtr handle) override;
  private:
    fid_poll *pollset;
    fi_poll_attr attr = {};
    LogPtr logger;
};

#endif
