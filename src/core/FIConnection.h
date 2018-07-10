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

#include "HPNL/Connection.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/Common.h"
#include "util/Ptr.h"
#include "demultiplexer/Handle.h"

class FIConnection : public Connection {
  public:
    FIConnection(fid_fabric*, fi_info*, fid_domain*, fid_cq*, fid_wait*, BufMgr*, BufMgr*, bool);
    ~FIConnection();

    virtual void read(char*, int) override;
    virtual void write(char*, int, int) override;
    virtual void shutdown() override;
    
    void connect();
    void accept();

    HandlePtr get_eqhandle();
    fid* get_fid();
    void activate_chunk(Chunk*);

    void set_read_callback(Callback*);
    void set_send_callback(Callback*);
    void set_shutdown_callback(Callback*);
    Callback* get_read_callback();
    Callback* get_send_callback();
    Callback* get_shutdown_callback();
    
  private:
    fi_info *info;
    fid_domain *domain;
    fid_ep *ep;
    fid_cq *conCq;
    fid_eq *conEq;

    uint64_t mid;
    BufMgr *recv_buf_mgr;
    BufMgr *send_buf_mgr;

    HandlePtr cqHandle;
    HandlePtr eqHandle;

    fid_wait *waitset;

    bool server;

    Callback* read_callback;
    Callback* send_callback;
    Callback* shutdown_callback;
};

#endif
