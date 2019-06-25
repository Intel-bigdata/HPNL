#ifndef STACK
#define STACK

#include "HPNL/BufMgr.h"

struct fid_eq;

class Stack {
  public:
    virtual ~Stack() = default;
    virtual int init() = 0;
    virtual void* bind(const char*, const char*, BufMgr*) = 0;
};

#endif
