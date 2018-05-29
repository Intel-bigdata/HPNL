#ifndef CONNECTION_H
#define CONNECTION_H

#include <memory>

class Connection {
  public:
    virtual ~Connection() {}
    virtual void read(char *buffer, int buffer_size) = 0;
    virtual void write(char *buffer, int buffer_size) = 0;
    virtual void shutdown() = 0;
};

#endif
