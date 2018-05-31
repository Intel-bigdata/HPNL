#ifndef CONNECTION_H
#define CONNECTION_H

#include <memory>

#include "util/Callback.h"

class Connection {
  public:
    virtual ~Connection() {}
    virtual void read(char *buffer, int buffer_size) = 0;
    virtual void write(char *buffer, int buffer_size) = 0;
    virtual void shutdown() = 0;
    virtual void set_read_callback(Callback*) = 0;
    virtual Callback* get_read_callback() = 0;
};

#endif
