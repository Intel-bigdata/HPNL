#ifndef STACK
#define STACK

#include "HPNL/ChunkMgr.h"

class Stack {
  public:
    virtual ~Stack() = default;
    virtual int init() = 0;
    virtual void* bind(const char*, const char*, ChunkMgr*) = 0;
};

#endif
