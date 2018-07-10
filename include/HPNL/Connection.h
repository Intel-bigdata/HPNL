#ifndef CONNECTION_H
#define CONNECTION_H

class Connection {
  public:
    virtual void read(char*, int) {}
    virtual void write(char*, int, int) {}
    virtual void shutdown() {}
};

#endif
