#ifndef RDMCONNECTION_H
#define RDMCONNECTION_H

#include <rdma/fi_domain.h>
#include <rdma/fabric.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_endpoint.h>
#include <string.h>

#include <vector>
#include <map>

#include "HPNL/Connection.h"
#include "HPNL/BufMgr.h"

class RdmConnection : public Connection {
  public:
    RdmConnection(const char*, const char*, fi_info*, fid_domain*, fid_cq*, BufMgr*, BufMgr*, int, bool);
    ~RdmConnection();
    virtual int init() override;
    virtual int send(Chunk*) override;
    virtual char* get_peer_name() override;
    fid_cq* get_cq();
    virtual void reclaim_chunk(Chunk*) override;
    virtual int activate_chunk(Chunk*) override;

    virtual void set_recv_callback(Callback*) override;
    virtual void set_send_callback(Callback*) override;
    virtual Callback* get_recv_callback() override;
    virtual Callback* get_send_callback() override;

    virtual void decode_peer_name(void*, char*) override;
    virtual char* decode_buf(void *buf) override;
    virtual Chunk* encode(void *buf, int size, char*) override;
  private:
    fid_fabric *fabric;
    fi_info *info;
    fid_domain *domain;
    fid_ep *ep;
    fid_av *av;
    fid_cq *conCq;
    fid_eq *conEq;
    
    const char* ip;
    const char* port;
    char local_name[64];
    size_t local_name_len = 64;
    std::map<std::string, fi_addr_t> addr_map;

    BufMgr *rbuf_mgr;
    BufMgr *sbuf_mgr;
    std::vector<Chunk*> recv_buffers;
    std::vector<Chunk*> send_buffers;
    int buffer_num;
    bool is_server;

    Callback* recv_callback;
    Callback* send_callback;

    bool inited = false;
};

#endif
