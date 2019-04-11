#ifndef SERVICE
#define SERVICE

#include <string>

#include "HPNL/Common.h"
#include "HPNL/Callback.h"
#include "HPNL/FiStack.h"
#include "HPNL/FiConnection.h"
#include "HPNL/ConMgr.h"
#include "HPNL/Proactor.h"
#include "HPNL/EqHandler.h"
#include "HPNL/EqDemultiplexer.h"
#include "HPNL/CqDemultiplexer.h"

class AcceptRequestCallback;

class Service {
  public:
    void run(const char*, const char*, int, int);
    void shutdown();
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
    Service(bool is_server_ = false);
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
    ConMgr *conMgr;
    EqDemultiplexer *eq_demulti_plexer;
    CqDemultiplexer *cq_demulti_plexer[MAX_WORKERS];
    Proactor *proactor;

    EQThread *eqThread;
    CQThread *cqThread[MAX_WORKERS];
    EventThread *eventThread;
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
