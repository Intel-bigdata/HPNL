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
  int rdma_buffer_id;
  int block_buffer_id;
  long seq;
};

class BufMgr {
  public:
    virtual ~BufMgr() {}

    // not thread safe
    virtual Chunk* index(int id) = 0;
    virtual void add(int, Chunk*) = 0;
    virtual Chunk* get() = 0;

    int get_id() { return id++; }
  private:
    int id = 0;
};

#endif
