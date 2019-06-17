#ifndef RDMSTACK_H
#define RDMSTACK_H

#include <string.h>
#include <assert.h>

#include <thread>
#include <atomic>
#include <mutex>
#include <map>
#include <vector>

#include "HPNL/BufMgr.h"
#include "core/Stack.h"
class RdmConnection;
class RdmStack : public Stack {
  public:
    RdmStack(int, bool, const char*);
    ~RdmStack();
    virtual int init() override;
    virtual void* bind(const char*, const char*, BufMgr*, BufMgr*) override;
    RdmConnection* get_con(const char*, const char*, BufMgr*, BufMgr*);
    fid_fabric* get_fabric();
    fid_cq* get_cq();

    RdmConnection* get_connection(long id);
    void reap(long id);

  private:
    fi_info *info;
    fi_info *server_info;
    fid_fabric *fabric;
    fid_domain *domain;
    fid_cq *cq = NULL;
    int buffer_num;
    bool is_server;

    std::map<long, RdmConnection*> conMap;
    std::mutex conMtx;
    std::atomic_long id_generator;

    std::mutex mtx;

    RdmConnection *server_con;

    const char* prov_name;
};

#endif
