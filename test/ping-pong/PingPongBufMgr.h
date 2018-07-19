#include "HPNL/BufMgr.h"

#include <vector>
#include <map>
#include <iostream>

class PingPongBufMgr : public BufMgr {
  public:
    PingPongBufMgr() {}
    virtual ~PingPongBufMgr() {
      for (auto buf : buf_map) {
        delete buf.second; 
        buf.second = NULL;
      } 
    }
    virtual Chunk* index(int id) override {
      return buf_map[id];
    }
    virtual void add(int mid, Chunk* ck) override {
      if (!buf_map.count(mid))
        buf_map[mid] = ck;
      bufs.push_back(ck);
    }

    virtual Chunk* get() override {
      Chunk *ck = bufs.back();
      bufs.pop_back();
      return ck;
    }
  private:
    std::vector<Chunk*> bufs;
    std::map<int, Chunk*> buf_map;
};

