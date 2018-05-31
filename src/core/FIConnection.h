#ifndef FICONNECTION_H
#define FICONNECTION_H

#include <string.h>

#include <rdma/fi_domain.h>
#include <rdma/fabric.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_endpoint.h>

#include <memory>

#include "core/Connection.h"
#include "util/Ptr.h"
#include "demultiplexer/Handle.h"
#include "util/Common.h"
#include "util/Mempool.h"

class FIConnection : public Connection {
  public:
    FIConnection(fid_fabric *fabric_, fi_info *info_, fid_domain *domain_, fid_cq *cq_, fid_wait *waitset_, Mempool *rpool, Mempool *spool, bool is_server);
    virtual ~FIConnection() override;
    virtual void read(char *buffer, int buffer_size) override;
    virtual void write(char *buffer, int buffer_size) override;
    virtual void set_read_callback(Callback*) override;
    virtual Callback* get_read_callback() override;

    void connect();
    void accept();
    void shutdown() override;
    HandlePtr connected();
    fid* get_fid();
    HandlePtr get_eqhandle();
    Mempool* get_rpool();
    Mempool* get_spool();
    void send_chunk_to_pool(std::vector<Chunk*>);
    void reactivate_chunk(Chunk *ck);
    
    bool active;
  private:
    Mempool *recv_pool;
    Mempool *send_pool;

    fi_info *info;
    fid_domain *domain;
    fid_ep *ep;
    fid_cq *conCq;
    fid_eq *conEq;

    HandlePtr cqHandle;
    HandlePtr eqHandle;

    fid_wait *waitset;

    bool server;

    Callback* read_callback;
};

#endif
