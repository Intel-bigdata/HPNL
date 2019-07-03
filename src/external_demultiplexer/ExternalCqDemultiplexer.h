#ifndef CQEXTERNALMULTIPLEXER_H
#define CQEXTERNALMULTIPLEXER_H

#ifdef __linux__
#include <sys/epoll.h>
#endif
#include <unistd.h>
#include <rdma/fi_cm.h>

#include "HPNL/ChunkMgr.h"

class MsgStack;

class ExternalCqDemultiplexer {
  public:
    ExternalCqDemultiplexer(MsgStack*, fid_cq*);
    ~ExternalCqDemultiplexer();
    int init();
    int wait_event(fid_eq**, Chunk**, int*, int*);
  private:
    MsgStack *stack;
    fid_cq *cq;
    uint64_t start;
    uint64_t end;
    #ifdef __linux__
    fid_fabric *fabric;
    struct epoll_event event;
    int epfd;
    int fd;
    #endif
};

#endif
