#ifndef SERVER_H
#define SERVER_H

#include "HPNL/Service.h"

class Server : public Service {
  public:
    Server(int, int);
    int init(bool msg = true);
    void start();
    int listen(const char*, const char*);
    void shutdown();
    void shutdown(Connection*);
    void wait();
    void set_recv_buf_mgr(BufMgr*);
    void set_send_buf_mgr(BufMgr*);
    void set_recv_callback(Callback *callback);
    void set_send_callback(Callback *callback);
    void set_read_callback(Callback *callback);
    void set_connected_callback(Callback *callback);
    uint64_t reg_rma_buffer(char*, uint64_t, int);
    void unreg_rma_buffer(int);
    Chunk* get_rma_buffer(int);
};

#endif
