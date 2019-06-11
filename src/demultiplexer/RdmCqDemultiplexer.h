#ifndef RDMCQDEMLUTIPLEXER_H
#define RDMCQDEMLUTIPLEXER_H
#include sysepoll.h
#include unistd.h
#include rdmafabric.h
#include rdmafi_domain.h

#include HPNLBufMgr.h

class RdmStack;

class RdmCqDemultiplexer {
  public
    RdmCqDemultiplexer(RdmStack);
    ~RdmCqDemultiplexer();
    int init();
    int wait_event();
  private
    RdmStack stack;
    int epfd;
    int fd;
    struct epoll_event event;
    fid_fabric fabric;
    fid_cq cq;
};
#endif