#ifndef CQDEMLUTIPLEXER_H
#define CQDEMLUTIPLEXER_H

#include <sys/epoll.h>
#include <unistd.h>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>

class MsgStack;

class CqDemultiplexer {
  public:
    CqDemultiplexer(MsgStack*, int);
    ~CqDemultiplexer();
    int init();
    int wait_event();
  private:
    MsgStack *stack;
    int work_num;
    int epfd;
    int fd;
    struct epoll_event event;
    fid_fabric *fabric;
    fid_cq *cq;
};

#endif
