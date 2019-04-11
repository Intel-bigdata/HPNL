#ifndef EQEXTERNALMULTIPLEXER_H
#define EQEXTERNALMULTIPLEXER_H

#include <sys/epoll.h>
#include <unistd.h>

#include <unordered_map>

#include "HPNL/FiStack.h"
#include "HPNL/FiConnection.h"
#include "HPNL/Common.h"

class ExternalEqDemultiplexer {
  public:
    ExternalEqDemultiplexer(FiStack*);
    ~ExternalEqDemultiplexer();
    int init();
    int wait_event(fi_info**, fid_eq**, FiConnection**);
    int add_event(fid_eq*);
    int delete_event(fid_eq*);
  private:
    FiStack *stack;
    fid_fabric *fabric;
    struct epoll_event event;
    int epfd;
    std::mutex mtx;

    std::unordered_map<fid*, fid_eq*> fid_map;
};

#endif
