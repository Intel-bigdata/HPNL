#ifndef BUFMGR_H
#define BUFMGR_H

#include <memory>

enum ChunkType {
  RECV_CHUNK = 1,
  SEND_CHUNK = 2
};

struct Chunk {
  ~Chunk() {
    buffer = NULL;
    mr = NULL;
    con = NULL;
  }
  void *buffer;
  void *mr;
  void *con;
  uint64_t capacity;
  int rdma_buffer_id;
};

class BufMgr {
  public:
    virtual ~BufMgr() {}

    // not thread safe
    virtual Chunk* index(int id) = 0;
    virtual void add(int, Chunk*) = 0;
    virtual Chunk* get() = 0;
    virtual int free_size() = 0;

    int get_id() { return id++; }
  private:
    int id = 0;
};

#endif
