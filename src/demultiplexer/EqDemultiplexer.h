#ifndef EQEVENTDEMULTIPLEXER_H
#define EQEVENTDEMULTIPLEXER_H

#include <atomic>
#include <map>
#include <memory>

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>

class Handle;
class EventHandler;

#define MAX_POLL_CNT 8

class EqDemultiplexer {
  public:
    EqDemultiplexer();
    ~EqDemultiplexer();
    int wait_event(std::map<std::shared_ptr<Handle>, std::shared_ptr<EventHandler>> eventMap);
    int register_event(std::shared_ptr<Handle>);
    int remove_event(std::shared_ptr<Handle>);
    void shutdown();
  private:
    int con_inflight = 0;
    std::atomic<bool> done{false};
};

#endif
