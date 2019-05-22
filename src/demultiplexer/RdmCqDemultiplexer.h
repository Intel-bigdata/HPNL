#ifndef RDMCQDEMLUTIPLEXER_H
#define RDMCQDEMLUTIPLEXER_H

#include <sys/epoll.h>
#include <unistd.h>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>

class RdmStack;

class RdmCqDemultiplexer {
  public:
    RdmCqDemultiplexer(RdmStack*);
    ~RdmCqDemultiplexer();
    int init();
    int wait_event();
  private:
    RdmStack *stack;
    int epfd;
    int fd;
    struct epoll_event event;
    fid_fabric *fabric;
    fid_cq *cq;
};

#endif

