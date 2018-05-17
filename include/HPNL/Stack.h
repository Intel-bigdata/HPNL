#ifndef STACK_H
#define STACK_H

#include "Ptr.h"
#include "Handle.h"
#include "Connection.h"

class Stack {
  public:
    virtual ~Stack() {}
    virtual HandlePtr bind() = 0;
    virtual void listen() = 0;
    virtual HandlePtr connect() = 0;
    virtual HandlePtr accept(void*) = 0;
    virtual void shutdown() = 0;
    virtual void reap(void*) = 0;
    virtual Connection* get_connection(fid* id) = 0;
    virtual void* get_domain() { return NULL; }
};

#endif
