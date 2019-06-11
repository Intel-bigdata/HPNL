#ifndef CONNECTION_H
#define CONNECTION_H

#include "HPNL/BufMgr.h"

class Connection {
  public:
    virtual int init() { return 0; }
    virtual void recv(char*, int) {}
    virtual int send(Chunk*) { return 0; }
    virtual int send(int, int) { return 0; }
	virtual int sendBuf(const char*, int) { return 0; }
    virtual int sendTo(int, int, const char*) { return 0; }
    virtual int sendBufTo(const char*, int, const char*) { return 0; }
    virtual int read(int, int, uint64_t, uint64_t, uint64_t) { return 0; }
    virtual void shutdown() { }
    virtual void take_back_chunk(Chunk*) {}
    virtual int activate_chunk(Chunk*) { return 0; }
    virtual int activate_chunk(int) {return 0;}
    virtual int get_cq_index() { return 0; }
    virtual void set_cq_index(int) {}
    virtual long get_id() {return 0;}
};

#endif
