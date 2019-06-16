#ifndef BUFMGR_H
#define BUFMGR_H

#include <rdma/fabric.h>

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
  uint32_t size;
  void *mr;
  void *con;
  uint32_t capacity;
  int buffer_id;
  fi_context2 ctx;
  fi_addr_t peer_addr;
};

class BufMgr {
  public:
    virtual ~BufMgr() {}

    // not thread safe
    virtual Chunk* get(int id) = 0;
    virtual void put(int, Chunk*) = 0;
    virtual Chunk* get() = 0;
    virtual int free_size() = 0;

    int get_id() { return id++; }
  private:
    int id = 0;
};

#endif
