#ifndef CQEXTERNALMULTIPLEXER_H
#define CQEXTERNALMULTIPLEXER_H

#include <sys/epoll.h>
#include <unistd.h>

#include "HPNL/FIStack.h"
#include "HPNL/FIConnection.h"

class CQExternalDemultiplexer {
  public:
    CQExternalDemultiplexer(FIStack*, fid_cq*);
    ~CQExternalDemultiplexer();
    int wait_event(fid_eq**, int*, int*);
  private:
    fid_fabric *fabric;
    struct epoll_event event;
    int epfd;
    int fd;
    fid_cq *cq;
    uint64_t start;
    uint64_t end;
};

#endif
