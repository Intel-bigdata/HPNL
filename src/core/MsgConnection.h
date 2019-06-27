#ifndef MSGCONNECTION_H
#define MSGCONNECTION_H

#include <string.h>

#include <rdma/fi_domain.h>
#include <rdma/fabric.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_rma.h>

#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "HPNL/Connection.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"

enum ConStatus {
  IDLE = 0,
  CONNECT_REQ,
  ACCEPT_REQ,
  CONNECTED,
  SHUTDOWN_REQ,
  DOWN
};

class MsgStack;

class MsgConnection : public Connection {
  public:
    MsgConnection(MsgStack*, fid_fabric*, fi_info*, fid_domain*, fid_cq*, BufMgr*, bool, int, int);
    ~MsgConnection() override;

    int init() override;
    int sendBuf(const char*, int) override;
    int send(int, int) override;
    int read(int, int, uint64_t, uint64_t, uint64_t) override;
    void activate_send_chunk(Chunk*) override;
    int activate_recv_chunk(Chunk*) override;
    
    int shutdown() override;
    int connect();
    int accept();

    void init_addr();
    void get_addr(char**, size_t*, char**, size_t*);
    int get_cq_index();
    fid_eq* get_eq();

    fid* get_fid();

    void set_recv_callback(Callback*) override;
    void set_send_callback(Callback*) override;
    void set_read_callback(Callback*);
    void set_shutdown_callback(Callback*);

    std::vector<Chunk*> get_send_chunks();

    Callback* get_recv_callback() override;
    Callback* get_send_callback() override;
    Callback* get_read_callback();
    Callback* get_shutdown_callback();

  public:
    ConStatus status;
    std::mutex con_mtx;
    std::condition_variable con_cv;
    
  private:
    MsgStack *stack;
    fid_fabric *fabric;
    fi_info *info;
    fid_domain *domain;
    fid_ep *ep;
    fid_cq *conCq;
    fid_eq *conEq;

    BufMgr *buf_mgr;
    std::vector<Chunk*> recv_chunks;
    std::vector<Chunk*> send_chunks;
    std::unordered_map<int, Chunk*> send_chunks_map;

    bool is_server;

    int buffer_num;
    int cq_index;

    size_t dest_port;
    char dest_addr[20];
    size_t src_port;
    char src_addr[20];

    Callback* recv_callback;
    Callback* send_callback;
    Callback* read_callback;
    Callback* shutdown_callback;
};

#endif
