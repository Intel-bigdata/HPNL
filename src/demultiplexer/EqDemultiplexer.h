#ifndef EQEVENTDEMULTIPLEXER_H
#define EQEVENTDEMULTIPLEXER_H

#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <unistd.h>

#include <atomic>
#include <map>
#include <memory>
#include <unordered_map>
#include <string.h>

#include <rdma/fabric.h>
#include <rdma/fi_cm.h>

class EventHandler;
class MsgStack;

class EqDemultiplexer {
  public:
    explicit EqDemultiplexer(MsgStack*);
    ~EqDemultiplexer();
    int init();
    int wait_event(std::map<fid*, std::shared_ptr<EventHandler>> eventMap);
    int register_event(fid*);
    int remove_event(fid*);
  private:
    MsgStack *stack;
    #ifdef __linux__
    fid_fabric *fabric;
    int epfd;
    struct epoll_event event;
    #endif
};

#endif
