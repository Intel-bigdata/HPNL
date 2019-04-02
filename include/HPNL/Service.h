#ifndef SERVICE
#define SERVICE

#include <string>

#include "HPNL/Common.h"
#include "HPNL/Callback.h"
#include "HPNL/FIStack.h"
#include "HPNL/FIConnection.h"
#include "HPNL/ConMgr.h"
#include "HPNL/Reactor.h"
#include "HPNL/EQHandler.h"
#include "HPNL/EQEventDemultiplexer.h"
#include "HPNL/CQEventDemultiplexer.h"

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
    void set_connected_callback(Callback*);
    void set_shutdown_callback(Callback*);
  protected:
    Service(bool is_server_ = false);
    ~Service();
  private:
    friend class AcceptRequestCallback;
    BufMgr *recvBufMgr;
    BufMgr *sendBufMgr;

    Callback *recvCallback;
    Callback *sendCallback;
    Callback *acceptRequestCallback;
    Callback *connectedCallback;
    Callback *shutdownCallback;

    int worker_num;
    bool is_server;

    FIStack *stack;
    ConMgr *conMgr;
    EQEventDemultiplexer *eq_demulti_plexer;
    CQEventDemultiplexer *cq_demulti_plexer[MAX_WORKERS];
    Reactor *reactor;

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
