#ifndef RDMCQDEMLUTIPLEXER_H
#define RDMCQDEMLUTIPLEXER_H

#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <unistd.h>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>

#include <HPNL/ChunkMgr.h>

class RdmStack;

class RdmCqDemultiplexer {
  public:
    RdmCqDemultiplexer(RdmStack*);
    ~RdmCqDemultiplexer();
    int init();
    int wait_event();
  private:
    RdmStack *stack;
    fid_cq *cq;
    #ifdef __linux__
    fid_fabric *fabric;
    struct epoll_event event;
    int epfd;
    int fd;
    #endif
};

#endif

