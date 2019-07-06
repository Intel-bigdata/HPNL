#ifndef SERVER_H
#define SERVER_H

#include <cstdint>

#include "FabricService.h"

class Service;
class Connection;
class ChunkMgr;
class Callback;
class Chunk;
class fid_domain;

class Server : public FabricService {
  public:
    // Worker is message handling thread.
    // Initial buffer is the buffer originally allocated to transfer send/receive msg.
    Server(int worker_number = 1, int initial_buffer_number = 16);
    ~Server();

    // Connection management
    // HPNL supports connection(MSG) and non-connection(RDM) endpoints.
    // If msg_type is set to true, client will be initialized as connection(MSG) endpoint;
    // otherwise, client will be initialized as non-connection(RDM) endpoint.
    // Default type is connection endpoint.
    int init(bool msg = true);
    // For connection endpoint.
    int listen(const char*, const char*);
    void shutdown();
    void shutdown(Connection*);

    // Service management
    void start();
    void wait();

    // Buffer management
    // This method should be called after init
    void set_buf_mgr(ChunkMgr*);

    // Initialize event callback
    void set_recv_callback(Callback *callback);
    void set_send_callback(Callback *callback);
    void set_read_callback(Callback *callback);
    void set_connected_callback(Callback *callback);
    void set_shutdown_callback(Callback *callback);

    // RMA buffer registration
    uint64_t reg_rma_buffer(char*, uint64_t, int);
    void unreg_rma_buffer(int);
    Chunk* get_rma_buffer(int);

    // For RDMA memory registration
    fid_domain* get_domain() override;
  private:
    Service *service;
};

#endif
