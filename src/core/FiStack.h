#ifndef FISTACK_H
#define FISTACK_H

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_cm.h>

#include <map>
#include <assert.h>
#include <mutex>

#include "HPNL/BufMgr.h"

class FiConnection;

#define MAX_WORKER_NUM 10

class FiStack {
  public:
    FiStack(uint64_t, int, int, bool);
    ~FiStack();
    int init();
    fid_eq* bind(const char*, const char*);
    int listen();
    fid_eq* connect(const char*, const char*, BufMgr*, BufMgr*);
    fid_eq* accept(void*, BufMgr*, BufMgr*);
    uint64_t reg_rma_buffer(char*, uint64_t, int);
    void unreg_rma_buffer(int);
    Chunk* get_rma_chunk(int);
    void shutdown();
    void reap(void*);
    FiConnection* get_connection(fid* id);
    fid_fabric* get_fabric();
    fid_cq** get_cqs();

  private:
    uint64_t flags;
    int worker_num;
    int buffer_num;
    bool is_server;
    uint64_t seq_num;
    int total_buffer_num;
    fid_fabric *fabric;
    fid_domain *domain;
    fi_info *hints, *info;
    fi_info *hints_tmp, *info_tmp;
    fid_eq *peq;
    fid_pep *pep;

    std::map<fid*, FiConnection*> conMap;

    fid_cq *cqs[MAX_WORKER_NUM];
    fid_wait *waitset;

    std::map<int, Chunk*> chunkMap;
    std::mutex mtx;
};

#endif
