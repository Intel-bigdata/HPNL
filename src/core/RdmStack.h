#ifndef RDMSTACK_H
#define RDMSTACK_H

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <string.h>
#include <assert.h>

#include <thread>
#include <mutex>

#include "HPNL/BufMgr.h"
#include "core/Stack.h"

class RdmConnection;

class RdmStack : public Stack {
  public:
    RdmStack(int, bool);
    ~RdmStack();
    virtual int init() override;
    virtual void* bind(const char*, const char*, BufMgr*, BufMgr*) override;

    RdmConnection* get_con(const char*, const char*, BufMgr*, BufMgr*);
    fid_fabric* get_fabric();
    fid_cq* get_cq();
  private:
    fi_info *info;
    fi_info *server_info;
    fid_fabric *fabric;
    fid_domain *domain;
    fid_cq *cq = NULL;
    int buffer_num;
    bool is_server;

    std::mutex mtx;

    RdmConnection *server_con;
};

#endif
