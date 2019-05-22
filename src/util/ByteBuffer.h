#ifndef BYTEBUFFER_H_
#define BYTEBUFFER_H_

#include <cstddef>

class ByteBuffer {
  public:
    ByteBuffer(int);
    ~ByteBuffer();

    size_t get_size();
    void   position(int);
    void   limit(int);
    void   reset();
    char*  get_bytes();
    char   get();
  private:
    char* buff;
    int   size;
    int   pos;
    int   limit_len;
};

#endif
