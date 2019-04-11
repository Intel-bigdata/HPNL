#include "HPNL/BufMgr.h"

#include <vector>
#include <map>
#include <iostream>
#include <mutex>

class ExternalEqServiceBufMgr : public BufMgr {
  public:
    ExternalEqServiceBufMgr() {}
    virtual ~ExternalEqServiceBufMgr() {
      for (auto buf : buf_map) {
        delete buf.second; 
        buf.second = NULL;
      } 
    }
    virtual Chunk* get(int id) override {
      std::lock_guard<std::mutex> l(mtx);
      return buf_map[id];
    }
    virtual void put(int mid, Chunk* ck) override {
      std::lock_guard<std::mutex> l(mtx);
      if (!buf_map.count(mid))
        buf_map[mid] = ck;
      bufs.push_back(ck);
    }
    virtual Chunk* get() override {
      std::lock_guard<std::mutex> l(mtx);
      Chunk *ck = bufs.back();
      bufs.pop_back();
      return ck;
    }
    virtual int free_size() override {
      std::lock_guard<std::mutex> l(mtx);
      return bufs.size(); 
    }
  private:
    std::mutex mtx;
    std::vector<Chunk*> bufs;
    std::map<int, Chunk*> buf_map;
};

