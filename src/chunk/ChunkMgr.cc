#include "HPNL/ChunkMgr.h"

#include <iostream>

char* PoolAllocator::malloc(const size_type bytes) {
  int chunk_size = buffer_size+sizeof(Chunk);
  int buffer_num = bytes/chunk_size;
  auto *memory = static_cast<Chunk*>(std::malloc(bytes));
  Chunk *ret = memory;
  for (int i = 0; i < buffer_num; i++) {
    memory->buffer = memory->data;
    memory->buffer_id = id++;
    memory->capacity = buffer_size;
    chunk_map[memory->buffer_id] = memory;
    memory = reinterpret_cast<Chunk*>(reinterpret_cast<char*>(memory)+chunk_size);
  }
  return reinterpret_cast<char *>(ret);
}

void PoolAllocator::free(char* const block) {
  std::cout << "pool allocator free." << std::endl;
  auto memory = reinterpret_cast<Chunk*>(block);
  std::free(memory);
}

int PoolAllocator::buffer_size = 0;
int PoolAllocator::id = 0;
std::map<int, Chunk*> PoolAllocator::chunk_map;
std::mutex PoolAllocator::mtx;

ChunkPool::ChunkPool(const int request_buffer_size,
  const int next_request_buffer_number, const int max_buffer_number) :
    pool(request_buffer_size+sizeof(Chunk), next_request_buffer_number, max_buffer_number),
    buffer_size(request_buffer_size) {
  PoolAllocator::buffer_size = buffer_size;
  PoolAllocator::id = 0;
}

ChunkPool::~ChunkPool() {
  PoolAllocator::chunk_map.clear();
  std::cout << "chunk pool destructor." << std::endl;
}

void* ChunkPool::malloc() {
  if (!store().empty()) {
    return (store().malloc());
  }
  return system_malloc();
}

void ChunkPool::free(void * const ck) {
  (store().free)(ck);
}

Chunk* ChunkPool::get(int id) {
  std::lock_guard<std::mutex> l(PoolAllocator::mtx);
  if (!PoolAllocator::chunk_map.count(id)) {
    std::cout << "request id " << id << " nullptr" << std::endl;
    return nullptr;
  }
  return PoolAllocator::chunk_map[id];
}

Chunk* ChunkPool::get() {
  return reinterpret_cast<Chunk*>(malloc());
}

void ChunkPool::put(int, Chunk*) {
  // pass
}

int ChunkPool::free_size() {
  return INT_MAX;
}

void* ChunkPool::system_malloc() {
  std::lock_guard<std::mutex> l(PoolAllocator::mtx);
  return boost::pool<PoolAllocator>::malloc();
}

DefaultChunkMgr::DefaultChunkMgr() : buffer_num(0), buffer_size(0), buffer_id(0) {}

DefaultChunkMgr::DefaultChunkMgr(int buffer_num_, uint64_t buffer_size_) : buffer_num(buffer_num_), buffer_size(buffer_size_), buffer_id(0) {
  for (int i = 0; i < buffer_num*2; i++) {
    auto ck = new Chunk();
    ck->buffer = std::malloc(buffer_size);
    ck->capacity = buffer_size;
    ck->buffer_id = this->get_id();
    this->put(ck->buffer_id, ck);
  }
}

DefaultChunkMgr::~DefaultChunkMgr() {
  for (auto buf : buf_map) {
    delete buf.second;
    buf.second = nullptr;
  }
}

Chunk* DefaultChunkMgr::get(int id) {
  std::lock_guard<std::mutex> l(mtx);
  return buf_map[id];
}

void DefaultChunkMgr::put(int mid, Chunk* ck) {
  std::lock_guard<std::mutex> l(mtx);
  if (!buf_map.count(mid))
    buf_map[mid] = ck;
  bufs.push_back(ck);
}

Chunk* DefaultChunkMgr::get() {
  std::lock_guard<std::mutex> l(mtx);
  if (bufs.empty())
    return nullptr;
  Chunk *ck = bufs.back();
  bufs.pop_back();
  return ck;
}

int DefaultChunkMgr::free_size() {
  std::lock_guard<std::mutex> l(mtx);
  return bufs.size();
}

uint32_t DefaultChunkMgr::get_id() {
  return buffer_id++;
}
