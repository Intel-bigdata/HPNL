#ifndef SERVICE
#define SERVICE

#include <assert.h>

#include "HPNL/Callback.h"
#include "HPNL/BufMgr.h"
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
    int init(bool msg_ = true);
    int listen(const char*, const char*);
    int connect(const char*, const char*);
    Connection* get_con(const char*, const char*);
    Stack* get_stack();
    void start();
    void shutdown();
    void shutdown(Connection *con);
    void wait();
    void set_recv_buf_mgr(BufMgr*);
    void set_send_buf_mgr(BufMgr*);

    void set_send_callback(Callback*);
    void set_recv_callback(Callback*);
    void set_read_callback(Callback*);
    void set_connected_callback(Callback*);
    void set_shutdown_callback(Callback*);

    uint64_t reg_rma_buffer(char*, uint64_t, int);
    void unreg_rma_buffer(int);
    Chunk* get_rma_buffer(int);
  protected:
    Service(int, int, bool is_server_ = false);
    ~Service();
  private:
    friend class AcceptRequestCallback;

    Service(const Service& service) {}
    Service& operator=(const Service &service) { return *this; }
    BufMgr *recvBufMgr;
    BufMgr *sendBufMgr;

    Callback *recvCallback;
    Callback *sendCallback;
    Callback *readCallback;
    Callback *acceptRequestCallback;
    Callback *connectedCallback;
    Callback *shutdownCallback;

    int worker_num;
    int buffer_num;
    bool is_server;
    bool msg;

    Stack *stack;
    EqDemultiplexer *eq_demulti_plexer;
    CqDemultiplexer *cq_demulti_plexer[MAX_WORKERS];
    RdmCqDemultiplexer *rdm_cq_demulti_plexer;
    Proactor *proactor;

    EqThread *eqThread;
    CqThread *cqThread[MAX_WORKERS];
    RdmCqThread *rdmCqThread;
};

class AcceptRequestCallback : public Callback {
  public:
    AcceptRequestCallback(Service *ioService_) : ioService(ioService_) {}
    virtual ~AcceptRequestCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      assert(ioService->recvBufMgr);
      assert(ioService->sendBufMgr);
      BufMgr **recvBufMgr = (BufMgr**)param_1;
      BufMgr **sendBufMgr = (BufMgr**)param_2;

      *recvBufMgr = ioService->recvBufMgr; 
      *sendBufMgr = ioService->sendBufMgr;
    }
  private:
    Service *ioService;
};

#endif
