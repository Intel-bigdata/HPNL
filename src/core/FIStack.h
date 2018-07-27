#ifndef FISTACK_H
#define FISTACK_H

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_cm.h>

#include <map>

#include "HPNL/BufMgr.h"
#include "core/FIConnection.h"
#include "core/ConMgr.h"
#include "demultiplexer/Handle.h"

class FIStack {
  public:
    FIStack(const char*, const char*, uint64_t, ConMgr*);
    ~FIStack();
    HandlePtr bind();
    void listen();
    HandlePtr connect(BufMgr*, BufMgr*);
    HandlePtr accept(void*, BufMgr*, BufMgr*);
    void shutdown();
    void reap(void*);
    FIConnection* get_connection(fid* id);
    fid_fabric* get_fabric();
    fid_cq** get_cqs();

  private:
    uint64_t seq_num;
    fid_fabric *fabric;
    fid_domain *domain;
    fi_info *hints, *info;
    fid_eq *peq;
    fid_pep *pep;

    std::map<fid*, FIConnection*> conMap;
    HandlePtr peqHandle;

    fid_cq *cqs[WORKERS];
    Handle *cqHandle[WORKERS];

    fid_wait *waitset;

    ConMgr *conMgr;
};

#endif
