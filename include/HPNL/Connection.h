#ifndef CONNECTION_H
#define CONNECTION_H

#include "HPNL/BufMgr.h"

class Connection {
  public:
    virtual void recv(char*, int) {}
    virtual void send(const char*, int, int, int, long) {}
    virtual void send(int, int) {}
    virtual void read(int, int, uint64_t, uint64_t, uint64_t) {}
    virtual void shutdown() {}
    virtual void take_back_chunk(Chunk*) {}
    virtual void activate_chunk(Chunk*) {}
};

#endif
