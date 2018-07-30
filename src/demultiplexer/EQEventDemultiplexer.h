#ifndef EQEVENTDEMULTIPLEXER_H
#define EQEVENTDEMULTIPLEXER_H

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>

#include "core/ConMgr.h"
#include "demultiplexer/EventDemultiplexer.h"
#include "demultiplexer/Handle.h"
#include "util/Ptr.h"

#define MAX_POLL_CNT 8

class EQEventDemultiplexer : public EventDemultiplexer {
  public:
    EQEventDemultiplexer(ConMgr*, bool);
    virtual ~EQEventDemultiplexer() override;
    virtual int wait_event(std::map<HandlePtr, EventHandlerPtr> &) override;
    virtual int register_event(HandlePtr) override;
    virtual int remove_event(HandlePtr) override;
  private:
    ConMgr *conMgr;
    bool is_server;
};

#endif
