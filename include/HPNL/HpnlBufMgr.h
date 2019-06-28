#include "HPNL/BufMgr.h"

#include <vector>
#include <map>
#include <iostream>
#include <mutex>

class HpnlBufMgr : public BufMgr {
  public:
    HpnlBufMgr(int buffer_num_, int buffer_size_) : buffer_num(buffer_num_), buffer_size(buffer_size_) {
      for (int i = 0; i < buffer_num*2; i++) {
        auto ck = new Chunk();
        ck->buffer_id = this->get_id();
        ck->buffer = std::malloc(buffer_size);
        ck->capacity = buffer_size;
        this->put(ck->buffer_id, ck);
      }
    }
    ~HpnlBufMgr() override {
      for (auto buf : buf_map) {
        delete buf.second; 
        buf.second = nullptr;
      } 
    }
    Chunk* get(int id) override {
      std::lock_guard<std::mutex> l(mtx);
      return buf_map[id];
    }
    void put(int mid, Chunk* ck) override {
      std::lock_guard<std::mutex> l(mtx);
      if (!buf_map.count(mid))
        buf_map[mid] = ck;
      bufs.push_back(ck);
    }
    Chunk* get() override {
      std::lock_guard<std::mutex> l(mtx);
      if (bufs.empty())
        return nullptr;
      Chunk *ck = bufs.back();
      bufs.pop_back();
      return ck;
    }
    int free_size() override {
      std::lock_guard<std::mutex> l(mtx);
      return bufs.size(); 
    }
  private:
    std::mutex mtx;
    std::vector<Chunk*> bufs;
    std::map<int, Chunk*> buf_map;
    int buffer_size;
    int buffer_num;
};

