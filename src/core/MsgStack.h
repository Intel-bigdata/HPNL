#ifndef MSGSTACK_H
#define MSGSTACK_H

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_cm.h>

#include <map>
#include <assert.h>
#include <mutex>

#include "HPNL/BufMgr.h"
#include "core/Stack.h"

class MsgConnection;

#define MAX_WORKER_NUM 10

class MsgStack : public Stack {
  public:
    MsgStack(uint64_t, int, int, bool);
    virtual ~MsgStack();
    virtual int init() override;
    virtual void* bind(const char*, const char*, BufMgr*, BufMgr*) override;
    int listen();
    fid_eq* connect(const char*, const char*, BufMgr*, BufMgr*);
    fid_eq* accept(void*, BufMgr*, BufMgr*);
    uint64_t reg_rma_buffer(char*, uint64_t, int);
    void unreg_rma_buffer(int);
    Chunk* get_rma_chunk(int);
    void shutdown();
    void reap(void*);
    MsgConnection* get_connection(fid* id);
    fid_fabric* get_fabric();
    fid_cq** get_cqs();

  private:
    MsgStack(const MsgStack& stack) {}
    MsgStack& operator=(const MsgStack& stack) { return *this; }
    uint64_t flags;
    int worker_num;
    int buffer_num;
    BufMgr* recv_buf_mgr;
    BufMgr* send_buf_mgr;
    bool is_server;
    uint64_t seq_num;
    int total_buffer_num;
    fid_fabric *fabric;
    fid_domain *domain;
    fi_info *hints, *info;
    fi_info *hints_tmp, *info_tmp;
    fid_eq *peq;
    fid_pep *pep;

    std::map<fid*, MsgConnection*> conMap;

    fid_cq *cqs[MAX_WORKER_NUM];
    fid_wait *waitset;

    std::map<int, Chunk*> chunkMap;
    std::mutex mtx;
};

#endif
