#ifndef EXTERNALSERVICE_H
#define EXTERNALSERVICE_H

#include <rdma/fi_domain.h>

#include "HPNL/FIStack.h"
#include "HPNL/Connection.h"
#include "HPNL/ExternalEqServiceBufMgr.h"
#include "HPNL/EQExternalDemultiplexer.h"
#include "HPNL/CQExternalDemultiplexer.h"

class ExternalEqService {
  public:
    ExternalEqService(int, int, bool is_server_ = false);
    ~ExternalEqService();
    int init();
    fid_eq* connect(const char*, const char*);
    fid_eq* accept(fi_info*);
    uint64_t reg_rma_buffer(char*, uint64_t, int);
    void unreg_rma_buffer(int);
    Chunk* get_rma_buffer(int);
    void set_recv_buffer(char* buffer, uint64_t size, int rdma_buffer_id);
    void set_send_buffer(char* buffer, uint64_t size, int rdma_buffer_id);

    int wait_eq_event(fi_info**, fid_eq**);
    int add_eq_event(fid_eq*);
    int delete_eq_event(fid_eq*);

    Connection* get_connection(fid_eq*);
    void reap(fid*);
    FIStack* get_stack();
    Chunk* get_chunk(int, int);
    int get_worker_num();
  private:
    FIStack *stack;

    int worker_num;
    int buffer_num;
    bool is_server;

    char *recvBuffer;
    char *sendBuffer;
    uint64_t recvSize;
    uint64_t sendSize;
    ExternalEqServiceBufMgr *recvBufMgr;
    ExternalEqServiceBufMgr *sendBufMgr;
    std::map<int, Chunk*> chunkMap;

    EQExternalDemultiplexer *eq_demulti_plexer;
};

#endif
