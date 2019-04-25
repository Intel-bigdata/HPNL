#ifndef FICONNECTION_H
#define FICONNECTION_H

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
#include "HPNL/Ptr.h"
#include "HPNL/Handle.h"

#include <jni.h>

enum ConStatus {
  IDLE = 0,
  CONNECT_REQ,
  ACCEPT_REQ,
  CONNECTED,
  SHUTDOWN_REQ,
  DOWN
};

class FIStack;

class FIConnection : public Connection {
  public:
    FIConnection(FIStack*, fid_fabric*, fi_info*, fid_domain*, fid_cq*, fid_wait*, BufMgr*, BufMgr*, bool, int buffer_num, int cq_index, long connect_id);
    ~FIConnection();

    virtual int init() override;
    virtual void recv(char*, int) override;
    virtual int send(const char*, int, int, int, long) override;
    virtual int send(int, int) override;
    virtual int read(int, int, uint64_t, uint64_t, uint64_t) override;
    virtual void shutdown() override;
    virtual void take_back_chunk(Chunk*) override;
    virtual int activate_chunk(Chunk*) override;
    virtual int activate_chunk(int rdmaBufferId) override;
    virtual int get_cq_index() override;
    virtual void set_cq_index(int index) override;
    virtual long get_id() override;
    
    int connect();
    int accept();

    void init_addr();
    void get_addr(char**, size_t*, char**, size_t*);

    HandlePtr get_eqhandle();
    fid* get_fid();

    void set_recv_callback(Callback*);
    void set_send_callback(Callback*);
    void set_shutdown_callback(Callback*);

    std::vector<Chunk*> get_send_buffer();

    Callback* get_read_callback();
    Callback* get_send_callback();
    Callback* get_shutdown_callback();

    jobject get_java_conn(){
    	return java_conn;
    }

    void set_java_conn(jobject java_conn_ ){
    	java_conn = java_conn_;
    }

    jmethodID get_callback_methodID(){
    	return callback_methodID;
    }

    void set_callback_methodID(jmethodID method){
    	callback_methodID = method;
    }

  public:
    ConStatus status;
    std::mutex con_mtx;
    std::condition_variable con_cv;
    
  private:
    FIStack *stack;
    fid_fabric *fabric;
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
    std::unordered_map<int, Chunk*> send_buffers_map;

    HandlePtr cqHandle;
    HandlePtr eqHandle;

    fid_wait *waitset;

    bool is_server;

    int buffer_num;

    int cq_index;
    long connect_id;

    size_t dest_port;
    char dest_addr[20];
    size_t src_port;
    char src_addr[20];

    Callback* read_callback;
    Callback* send_callback;
    Callback* shutdown_callback;

    jobject java_conn;
    jmethodID callback_methodID;
};

#endif
