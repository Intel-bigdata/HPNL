#ifndef SERVICE
#define SERVICE

#include <assert.h>

#include "HPNL/Callback.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Common.h"

class AcceptRequestCallback;
class FiStack;
class Proactor;
class EqDemultiplexer;
class CqDemultiplexer;
class EqHandler;
class EqThread;
class CqThread;
class Connection;

class Service {
  public:
    int init();
    int listen(const char*, const char*);
    int connect(const char*, const char*);
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
    BufMgr *recvBufMgr;
    BufMgr *sendBufMgr;

    Callback *recvCallback;
    Callback *sendCallback;
    Callback *readCallback;
    Callback *acceptRequestCallback;
    Callback *connectedCallback;
    Callback *shutdownCallback;

    int worker_num;
    bool is_server;

    FiStack *stack;
    EqDemultiplexer *eq_demulti_plexer;
    CqDemultiplexer *cq_demulti_plexer[MAX_WORKERS];
    Proactor *proactor;

    EqThread *eqThread;
    CqThread *cqThread[MAX_WORKERS];
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
