#ifndef EQEXTERNALMULTIPLEXER_H
#define EQEXTERNALMULTIPLEXER_H

#include <sys/epoll.h>
#include <unistd.h>

#include <unordered_map>

#include "HPNL/FIStack.h"
#include "HPNL/FIConnection.h"
#include "HPNL/Common.h"

class EQExternalDemultiplexer {
  public:
    EQExternalDemultiplexer(FIStack*);
    ~EQExternalDemultiplexer();
    int init();
    int wait_event(fi_info**, fid_eq**, FIConnection**);
    int add_event(fid_eq*);
    int delete_event(fid_eq*);
  private:
    void shutdown_con(fi_eq_cm_entry *entry, fid_eq **eq);

    FIStack *stack;
    fid_fabric *fabric;
    struct epoll_event event;
    int epfd;
    std::mutex mtx;

    std::unordered_map<fid*, fid_eq*> fid_map;
    std::mutex conMtx;
};

#endif
