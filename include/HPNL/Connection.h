#ifndef CONNECTION_H
#define CONNECTION_H

#include "HPNL/BufMgr.h"

class Connection {
  public:
    virtual int init() { return 0; }
    virtual void recv(char*, int) {}
    virtual int send(const char*, int, int, int, long) { return 0; }
    virtual int send(int, int) { return 0; }
    virtual int read(int, int, uint64_t, uint64_t, uint64_t) { return 0; }
    virtual void shutdown() { }
    virtual void take_back_chunk(Chunk*) {}
    virtual int activate_chunk(Chunk*) { return 0; }
};

#endif
