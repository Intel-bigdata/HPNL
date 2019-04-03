#ifndef CQEXTERNALMULTIPLEXER_H
#define CQEXTERNALMULTIPLEXER_H

#include <sys/epoll.h>
#include <unistd.h>

#include "HPNL/FIStack.h"
#include "HPNL/FIConnection.h"
#include "HPNL/Common.h"

class CQExternalDemultiplexer {
  public:
    CQExternalDemultiplexer(FIStack*, fid_cq*);
    ~CQExternalDemultiplexer();
    int init();
    int wait_event(fid_eq**, Chunk**, int*, int*);
  private:
    FIStack *stack;
    fid_cq *cq;
    fid_fabric *fabric;
    struct epoll_event event;
    int epfd;
    int fd;
    uint64_t start;
    uint64_t end;
};

#endif
