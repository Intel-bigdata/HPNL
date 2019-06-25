#ifndef CQDEMLUTIPLEXER_H
#define CQDEMLUTIPLEXER_H

#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <unistd.h>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>

class MsgStack;

class CqDemultiplexer {
  public:
    explicit CqDemultiplexer(MsgStack*, int);
    ~CqDemultiplexer();
    int init();
    int wait_event();
  private:
    MsgStack *stack;
    fid_cq *cq;
    int work_num;
    #ifdef __linux__
    int epfd;
    int fd;
    struct epoll_event event;
    fid_fabric *fabric;
    #endif
};

#endif
