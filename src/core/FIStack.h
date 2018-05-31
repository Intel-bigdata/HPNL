#ifndef FISTACK_H
#define FISTACK_H

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_cm.h>

#include <map>

#include "core/Stack.h"
#include "core/FIConnection.h"
#include "demultiplexer/Handle.h"
#include "util/Mempool.h"

class FIStack : public Stack {
  public:
    FIStack(const char *addr, const char *port, uint64_t flags);
    virtual ~FIStack() override;
    virtual HandlePtr bind() override;
    virtual void listen() override;
    virtual HandlePtr connect() override;
    virtual HandlePtr accept(void*) override;
    virtual void shutdown() override;
    virtual void reap(void*) override;
    FIConnection* get_connection(fid* id);

    HandlePtr connected(void *con_id);

    fid_fabric* get_fabric();
    fid_cq** get_cqs();
  private:
    Mempool *recv_pool;
    Mempool *send_pool;
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
};

#endif
