#ifndef EXTERNALRDMCQDEMLUTIPLEXER_H
#define EXTERNALRDMCQDEMLUTIPLEXER_H

#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <unistd.h>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>

#include <HPNL/BufMgr.h>

class RdmStack;

class ExternalRdmCqDemultiplexer {
  public:
    ExternalRdmCqDemultiplexer(RdmStack*);
    ~ExternalRdmCqDemultiplexer();
    int init();
    int wait_event(Chunk**, int*);
  private:
    RdmStack *stack;
    fid_cq *cq;
    uint64_t start;
    uint64_t end;
    #ifdef __linux__
    int epfd;
    int fd;
    struct epoll_event event;
    fid_fabric *fabric;
    #endif
};

#endif

