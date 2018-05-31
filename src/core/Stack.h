#ifndef STACK_H
#define STACK_H

#include "demultiplexer/Handle.h"
#include "util/Ptr.h"

class Stack {
  public:
    virtual ~Stack() {}
    virtual HandlePtr bind() = 0;
    virtual void listen() = 0;
    virtual HandlePtr connect() = 0;
    virtual HandlePtr accept(void*) = 0;
    virtual void shutdown() = 0;
    virtual void reap(void*) = 0;
};

#endif
