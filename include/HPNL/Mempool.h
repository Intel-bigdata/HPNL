#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <cstdlib>
#include <deque>
#include <vector>

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>

#include "Ptr.h"
#include "Common.h"

struct Chunk {
  uint32_t offset;
  fid_mr *mr;
  void* buffer;
};

class Mempool {
  public:
    Mempool(fid_domain *domain_, uint64_t size) : domain(domain_), nnext_size(size), index(0) {}
    Mempool(Mempool *pool, uint64_t size) : index(0) {
      take(pool, size);
      nnext_size = size;
    }
    std::vector<Chunk*> pop(uint64_t size) {
      std::vector<Chunk*> vec;
      for (int i = 0; i < size; i++) {
        vec.push_back(free_queue.back()); 
        free_queue.pop_back();
      }
      return std::move(vec);
    }
    std::vector<Chunk*> get(uint64_t size) {
      std::vector<Chunk*> vec;
      for (int i = 0; i < size; i++) {
        vec.push_back(free_queue[index++]);
        if (index >= nnext_size) {
          return std::move(vec); 
        }
      }
      return std::move(vec);
    }
    void take(Mempool *pool, uint64_t size) {
      std::vector<Chunk*> vec = pool->pop(size);
      push(std::move(vec));
    }
    void push(std::vector<Chunk*> vec) {
      for (auto ck : vec) {
        free_queue.push_back(ck); 
      }
      std::vector<Chunk*>().swap(vec);
    } 
    void release_memory() {
      for (auto ck : free_queue) {
        fi_close(&ck->mr->fid); 
        std::free(ck->buffer);
      }
    }
    void register_memory() {
      for (int i = 0; i < nnext_size; i++) {
        Chunk *ck = new Chunk();
        ck->buffer = (char*)std::malloc(BUFFER_SIZE); 
        fi_mr_reg(domain, ck->buffer, BUFFER_SIZE, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &ck->mr, NULL);
        free_queue.push_back(ck);
      } 
    }
  private:
    std::deque<Chunk*> free_queue;
    fid_domain *domain;
    uint64_t nnext_size;
    uint64_t index;
};

#endif
