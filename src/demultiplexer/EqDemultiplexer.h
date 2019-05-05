#ifndef EQEVENTDEMULTIPLEXER_H
#define EQEVENTDEMULTIPLEXER_H

#include <sys/epoll.h>
#include <unistd.h>

#include <atomic>
#include <map>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <string.h>

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>

class Handle;
class EventHandler;
class FiStack;

#define MAX_POLL_CNT 8

class EqDemultiplexer {
  public:
    EqDemultiplexer(FiStack*);
    ~EqDemultiplexer();
    int init();
    int wait_event(std::map<std::shared_ptr<Handle>, std::shared_ptr<EventHandler>> eventMap);
    int register_event(std::shared_ptr<Handle>);
    int remove_event(std::shared_ptr<Handle>);
    void shutdown();
  private:
    FiStack *stack;
    fid_fabric *fabric;
    struct epoll_event event;
    int epfd;
    std::mutex mtx;

    std::unordered_map<fid*, std::shared_ptr<Handle>> fid_map;
};

#endif
