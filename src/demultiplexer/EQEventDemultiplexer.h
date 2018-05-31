#ifndef EQEVENTDEMULTIPLEXER_H
#define EQEVENTDEMULTIPLEXER_H

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>

#include "demultiplexer/EventDemultiplexer.h"
#include "demultiplexer/Handle.h"
#include "util/Ptr.h"

#define MAX_POLL_CNT 8

class EQEventDemultiplexer : public EventDemultiplexer {
  public:
    EQEventDemultiplexer(LogPtr log_);
    virtual ~EQEventDemultiplexer() override;
    virtual int wait_event(std::map<HandlePtr, EventHandlerPtr> &eventMap) override;
    virtual int register_event(HandlePtr handle) override;
    virtual int remove_event(HandlePtr handle) override;
  private:
    LogPtr logger;
};

#endif
