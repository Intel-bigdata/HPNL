#ifndef SERVER_H
#define SERVER_H

#include "service/Service.h"

class Server : public Service {
  public:
    Server(const char*, const char*);
    void run(int);
    void shutdown();
    void wait();
    void set_recv_buf_mgr(BufMgr*);
    void set_send_buf_mgr(BufMgr*);
    void set_read_callback(Callback *callback);
    void set_send_callback(Callback *callback);
    void set_connected_callback(Callback *callback);
};

#endif
