#ifndef CQEXTERNALMULTIPLEXER_H
#define CQEXTERNALMULTIPLEXER_H

#include <sys/epoll.h>
#include <unistd.h>

#include "HPNL/FiStack.h"
#include "HPNL/FiConnection.h"
#include "HPNL/Common.h"

class ExternalCqDemultiplexer {
  public:
    ExternalCqDemultiplexer(FiStack*, fid_cq*);
    ~ExternalCqDemultiplexer();
    int init();
    int wait_event(fid_eq**, Chunk**, int*, int*);
  private:
    FiStack *stack;
    fid_cq *cq;
    fid_fabric *fabric;
    struct epoll_event event;
    int epfd;
    int fd;
    uint64_t start;
    uint64_t end;
};

#endif
