#ifndef CONNECTION_H
#define CONNECTION_H

#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"

class Connection {
  public:
    virtual int init() { return 0; }
    virtual fi_addr_t recv(const char*, int) { return 0; }
    virtual char* get_peer_name() { return 0; }
    virtual int send(const char*, int, long) { return 0; }
    virtual int send(Chunk*) { return 0; }
    virtual int send(int, int) { return 0; }
    virtual int read(int, int, uint64_t, uint64_t, uint64_t) { return 0; }
    virtual void decode_peer_name(void*, char*) {}
    virtual char* decode_buf(void *buf) { return nullptr; }
    virtual Chunk* encode(void *buf, int size, char*) { return nullptr; }

    virtual void reclaim_chunk(Chunk*) {}
    virtual int activate_chunk(Chunk*) { return 0; }
    virtual void set_recv_callback(Callback*) {}
    virtual void set_send_callback(Callback*) {}
    virtual Callback* get_recv_callback() { return nullptr; }
    virtual Callback* get_send_callback() { return nullptr; }
};

#endif
