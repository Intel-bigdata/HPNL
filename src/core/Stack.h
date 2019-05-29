#ifndef STACK
#define STACK

#include "HPNL/BufMgr.h"

class fid_eq;

class Stack {
  public:
    virtual int init() { return 0; }
    virtual void* bind(const char*, const char*, BufMgr*, BufMgr*) { return nullptr; }
    virtual int listen() { return 0; }
    virtual fid_eq* connect(const char*, const char*, BufMgr*, BufMgr*) { return nullptr; }
    virtual fid_eq* accept(void*, BufMgr*, BufMgr*) { return nullptr; }
};

#endif
