#ifndef CONNECTION_H
#define CONNECTION_H

#include "HPNL/BufMgr.h"

class Connection {
  public:
    virtual void read(char*, int) {}
    virtual void write(const char*, int, int) {}
    virtual void shutdown() {}
    virtual void take_back_chunk(Chunk*) {}
    virtual void activate_chunk(Chunk*) {}
};

#endif
