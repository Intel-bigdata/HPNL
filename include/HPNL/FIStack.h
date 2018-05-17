#ifndef FISTACK_H
#define FISTACK_H

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_cm.h>

#include <map>

#include "Stack.h"
#include "FIConnection.h"
#include "Handle.h"
#include "Mempool.h"

class FIStack : public Stack {
  public:
    FIStack(const char *addr, uint64_t flags);
    virtual ~FIStack() override;
    virtual HandlePtr bind() override;
    virtual void listen() override;
    virtual HandlePtr connect() override;
    virtual HandlePtr accept(void*) override;
    virtual void shutdown() override;
    virtual void reap(void*) override;
    virtual Connection* get_connection(fid* id) override;

    HandlePtr connected(void *con_id);

    virtual void* get_domain() override;
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
    HandlePtr pcmHandle;
};

#endif
