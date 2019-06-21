#ifndef CLIENT_H
#define CLIENT_H

#include "HPNL/Service.h"

class Client : public Service {
  public:
    Client(int, int);

    // Connection management
    int      init(bool msg = true);
    int      connect(const char*, const char*);
    void     shutdown();
    void     shutdown(Connection*);

    // Service management
    void     start();
    void     wait();

    // Buffer management
    void     set_buf_mgr(BufMgr*);

    // Initialize event callback
    void     set_send_buf_mgr(BufMgr*);
    void     set_recv_callback(Callback *callback);
    void     set_send_callback(Callback *callback);
    void     set_read_callback(Callback *callback);
    void     set_connected_callback(Callback *callback);

    // RMA buffer registration
    uint64_t reg_rma_buffer(char*, uint64_t, int);
    void     unreg_rma_buffer(int);
    Chunk*   get_rma_buffer(int);
};

#endif
