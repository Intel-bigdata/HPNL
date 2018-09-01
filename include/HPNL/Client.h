#ifndef CLIENT_H
#define CLIENT_H

#include "HPNL/Service.h"

class Client : public Service {
  public:
    Client(const char*, const char*);
    void run(int);
    void shutdown();
    void wait();
    void set_recv_buf_mgr(BufMgr*);
    void set_send_buf_mgr(BufMgr*);
    void set_recv_callback(Callback *callback);
    void set_send_callback(Callback *callback);
    void set_connected_callback(Callback *callback);
};

#endif
