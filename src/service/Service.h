#ifndef SERVICE
#define SERVICE

#include <string>

#include "HPNL/Callback.h"
#include "core/FIStack.h"
#include "core/FIConnection.h"
#include "core/ConMgr.h"
#include "demultiplexer/Reactor.h"
#include "demultiplexer/EQHandler.h"
#include "demultiplexer/EQEventDemultiplexer.h"
#include "demultiplexer/CQEventDemultiplexer.h"

class AcceptRequestCallback;

class Service {
  public:
    void run(int);
    void shutdown();
    void wait();
    void set_recv_buf_mgr(BufMgr*);
    void set_send_buf_mgr(BufMgr*);

    void set_send_callback(Callback*);
    void set_read_callback(Callback*);
    void set_connected_callback(Callback*);
    void set_shutdown_callback(Callback*);
  protected:
    Service(const char*, const char*, bool is_server_ = false);
    ~Service();
  private:
    friend class AcceptRequestCallback;
    BufMgr *recvBufMgr;
    BufMgr *sendBufMgr;

    Callback *readCallback;
    Callback *sendCallback;
    Callback *acceptRequestCallback;
    Callback *connectedCallback;
    Callback *shutdownCallback;

    int worker_num;
    const char* ip;
    const char* port;
    bool is_server;

    FIStack *stack;
    ConMgr *conMgr;
    EQEventDemultiplexer *eq_demulti_plexer;
    CQEventDemultiplexer *cq_demulti_plexer[WORKERS];
    Reactor *reactor;

    EQThread *eqThread;
    CQThread *cqThread[WORKERS];
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
