#ifndef FICONNECTION_H
#define FICONNECTION_H

#include <string.h>

#include <rdma/fi_domain.h>
#include <rdma/fabric.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_endpoint.h>

#include <memory>
#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "HPNL/Connection.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/Common.h"
#include "HPNL/ConMgr.h"
#include "HPNL/Ptr.h"
#include "HPNL/Handle.h"

enum ConStatus {
  IDLE = 0,
  CONNECT_REQ,
  ACCEPT_REQ,
  CONNECTED,
  SHUTDOWN_REQ,
  DOWN
};

class FIConnection : public Connection {
  public:
    FIConnection(fid_fabric*, fi_info*, fid_domain*, fid_cq*, fid_wait*, BufMgr*, BufMgr*, ConMgr*, bool);
    ~FIConnection();

    virtual void recv(char*, int) override;
    virtual void send(const char*, int, int, int, long) override;
    virtual void shutdown() override;
    virtual void take_back_chunk(Chunk*) override;
    virtual void activate_chunk(Chunk*) override;
    
    void connect();
    void accept();

    HandlePtr get_eqhandle();
    fid* get_fid();

    void set_recv_callback(Callback*);
    void set_send_callback(Callback*);
    void set_shutdown_callback(Callback*);
    Callback* get_read_callback();
    Callback* get_send_callback();
    Callback* get_shutdown_callback();

  public:
    ConStatus status;
    std::mutex con_mtx;
    std::condition_variable con_cv;
    
  private:
    fi_info *info;
    fid_domain *domain;
    fid_ep *ep;
    fid_cq *conCq;
    fid_eq *conEq;

    uint64_t mid;
    BufMgr *recv_buf_mgr;
    BufMgr *send_buf_mgr;
    std::vector<Chunk*> recv_buffers;
    std::vector<Chunk*> send_buffers;

    HandlePtr cqHandle;
    HandlePtr eqHandle;

    fid_wait *waitset;

    ConMgr *conMgr;
    bool server;

    Callback* read_callback;
    Callback* send_callback;
    Callback* shutdown_callback;
};

#endif
