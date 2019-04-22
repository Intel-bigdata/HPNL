#ifndef SERVER_H
#define SERVER_H

#include "HPNL/Service.h"

class Server : public Service {
  public:
    Server();
    void run(const char*, const char*, int, int, int);
    void shutdown();
    void wait();
    void set_recv_buf_mgr(BufMgr*);
    void set_send_buf_mgr(BufMgr*);
    void set_recv_callback(Callback *callback);
    void set_send_callback(Callback *callback);
    void set_connected_callback(Callback *callback);
};

#endif
