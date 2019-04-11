#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <unistd.h>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>

#include <thread>

#include "HPNL/FiConnection.h"
#include "HPNL/FiStack.h"

class CqDemultiplexer {
  public:
    CqDemultiplexer(FiStack*, int);
    ~CqDemultiplexer();
    int wait_event();
  private:
    int epfd;
    int fd;
    struct epoll_event event;
    fid_fabric *fabric;
    fid_cq *cq;
};

#endif
