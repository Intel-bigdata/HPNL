#include "HPNL/BufMgr.h"

#include <vector>
#include <map>
#include <iostream>

class PingPongBufMgr : public BufMgr {
  public:
    PingPongBufMgr() {
      iter = bufs.begin(); 
    }
    virtual ~PingPongBufMgr() {
      for (auto buf : buf_map) {
        delete buf.second; 
        buf.second = NULL;
      } 
    }
    virtual Chunk* begin() override {
      return *bufs.begin(); 
    }
    virtual Chunk* end() override {
      return *bufs.end();
    }
    virtual Chunk* index(int id) override {
      return buf_map[id];
    }
    virtual void add(int mid, Chunk* ck) override {
      if (!buf_map.count(mid))
        buf_map[mid] = ck;
      bufs.push_back(ck);
  }
    virtual Chunk* next() override {
      if (iter == bufs.end()) {
        return NULL;
      } else {
        Chunk *ck = *iter++;
        return ck;
      }
    }
    virtual Chunk* prev() override {
      Chunk *ck;
      if (iter == bufs.begin()) {
        return NULL;
      } else {
        return *(iter--);
      }
    }
    virtual void rewind() override {
      ck_ptr = begin();
      iter = bufs.begin();
    }

    virtual Chunk* get(int id) override {
      Chunk *ck = bufs.back();
      bufs.pop_back();
      return ck;
    }
  private:
    std::vector<Chunk*> bufs;
    std::vector<Chunk*>::iterator iter;
    std::map<int, Chunk*> buf_map;
    Chunk *ck_ptr;
};

