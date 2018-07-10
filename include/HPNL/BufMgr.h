#ifndef BUFMGR_H
#define BUFMGR_H

#include <memory>

struct Chunk {
  ~Chunk() {
    std::free(buffer);
    mr = NULL;
    con = NULL;
  }
  void *buffer;
  int mid;
  void *mr;
  void *con;
};

class BufMgr {
  public:
    virtual ~BufMgr() {}
    virtual Chunk* begin() = 0;
    virtual Chunk* end() = 0;
    virtual void rewind() = 0;
    virtual Chunk* next() = 0;
    virtual Chunk* prev() = 0;
    virtual Chunk* index(int id) = 0;
    virtual void add(int, Chunk*) = 0;
    virtual Chunk* get(int id) = 0;

    int get_id() { return id++; }
  private:
    int id = 0;
};

#endif
