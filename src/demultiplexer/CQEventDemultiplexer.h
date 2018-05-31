#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>

#include "core/FIConnection.h"

class CQEventDemultiplexer {
  public:
    CQEventDemultiplexer(fid_fabric *fabric_, fid_cq *cq_);
    ~CQEventDemultiplexer();
    int wait_event();
  private:
    int epfd;
    int fd;
    struct epoll_event event;
    fid_fabric *fabric;
    fid_cq *cq;
};

#endif
