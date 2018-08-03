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
    ExternalEqService(const char*, const char*, bool is_server_ = false);
    ~ExternalEqService();
    fid_eq* connect();
    fid_eq* accept(fi_info*);
    void set_recv_buffer(char* buffer, uint64_t size);
    void set_send_buffer(char* buffer, uint64_t size);

    int wait_eq_event(fid_eq*, fi_info**);

    Connection* get_connection(fid_eq*);
    FIStack* get_stack();
    Chunk* get_chunk(int, int);
  private:
    void prepare();
  private:
    FIStack *stack;

    int worker_num;
    const char* ip;
    const char* port;
    bool is_server;

    char *recvBuffer;
    char *sendBuffer;
    uint64_t recvSize;
    uint64_t sendSize;
    ExternalEqServiceBufMgr *recvBufMgr;
    ExternalEqServiceBufMgr *sendBufMgr;

    ConMgr *conMgr;

    EQExternalDemultiplexer *eq_demulti_plexer;
};

#endif
