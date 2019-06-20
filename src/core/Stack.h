#ifndef STACK
#define STACK

#include "HPNL/BufMgr.h"

class fid_eq;

class Stack {
  public:
    virtual ~Stack() {}
    virtual int init() { return 0; }
    virtual void* bind(const char*, const char*, BufMgr*, BufMgr*) { return nullptr; }
};

#endif
