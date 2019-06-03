#ifndef EXTERNALRDMCQDEMLUTIPLEXER_H
#define EXTERNALRDMCQDEMLUTIPLEXER_H

#include <sys/epoll.h>
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
    int epfd;
    int fd;
    struct epoll_event event;
    fid_fabric *fabric;
    fid_cq *cq;
    uint64_t start;
    uint64_t end;
};

#endif

