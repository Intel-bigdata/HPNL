#ifndef SERVICE
#define SERVICE

#include <assert.h>

#include "HPNL/Callback.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Common.h"

class AcceptRequestCallback;
class Stack;
class MsgStack;
class Proactor;
class EqDemultiplexer;
class CqDemultiplexer;
class RdmCqDemultiplexer;
class EqHandler;
class EqThread;
class CqThread;
class RdmCqThread;
class Connection;

class Service {
  public:
    Service(int, int, bool is_server_ = false);
    ~Service();
    Service(const Service& service) = delete;
    Service&           operator=(const Service &service) = delete;

    // Connection management
    int                init(bool msg_ = true);
    int                listen(const char*, const char*);
    int                connect(const char*, const char*);
    void               shutdown();
    void               shutdown(Connection *con);

    // Service management
    void               start();
    void               wait();

    // Initialize buffer container
    void               set_buf_mgr(ChunkMgr*);

    // Initialize event callback
    void               set_send_callback(Callback*);
    void               set_recv_callback(Callback*);
    void               set_read_callback(Callback*);
    void               set_connected_callback(Callback*);
    void               set_shutdown_callback(Callback*);

    // RMA buffer registration
    uint64_t           reg_rma_buffer(char*, uint64_t, int);
    void               unreg_rma_buffer(int);
    Chunk*             get_rma_buffer(int);

    // Other util functions
    Connection*        get_con(const char*, const char*);
    fid_domain*        get_domain();
  private:
    friend class       AcceptRequestCallback;

    Stack              *stack;
    Proactor           *proactor;
    EqDemultiplexer    *eq_demultiplexer;
    CqDemultiplexer    *cq_demultiplexer[MAX_WORKERS]{};
    RdmCqDemultiplexer *rdm_cq_demultiplexer;

    ChunkMgr             *bufMgr{};

    Callback           *recvCallback;
    Callback           *sendCallback;
    Callback           *readCallback;
    Callback           *acceptRequestCallback;
    Callback           *connectedCallback;
    Callback           *shutdownCallback;

    int                worker_num;
    int                buffer_num;
    bool               is_server;
    bool               msg;

    EqThread           *eqThread;
    CqThread           *cqThread[MAX_WORKERS]{};
    RdmCqThread        *rdmCqThread;
};

class AcceptRequestCallback : public Callback {
  public:
    explicit AcceptRequestCallback(Service *ioService_) : ioService(ioService_) {}
    ~AcceptRequestCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      assert(ioService->bufMgr);
      auto bufMgr = (ChunkMgr**)param_1;
      *bufMgr = ioService->bufMgr;
    }
  private:
    Service *ioService;
};

#endif
