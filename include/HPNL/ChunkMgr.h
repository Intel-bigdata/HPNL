#ifndef BUFMGR_H
#define BUFMGR_H

#include <rdma/fabric.h>

#include <memory>
#include <vector>
#include <map>
#include <mutex>

#include <boost/pool/pool.hpp>

struct Chunk {
  // The number of bytes that allocated in total
  uint64_t capacity;
  // The number of bytes that are valid
  uint64_t size;
  // A pointer to HPNL connection object
  void *con;
  // A pointer to RDMA memory_region
  void *mr;
  // Buffer unique id
  int buffer_id;
  // Libfabric context in chunk lifetime
  fi_context2 ctx{};
  // Peer endpoint's address when sending message
  fi_addr_t peer_addr{};
  // A pointer to a piece of contiguous memory.
  // Chunk class won't handle the lifetime of data
  void *buffer;
  // char placeholder for chunk buffer
  char data[0];
};

/*
 * Chunk manger is to manage free buffer list
 * All the function is not thread safe. That means, user application need to handle the race condition.
 */
class ChunkMgr {
  public:
    // Get one chunk from chunk manager
    virtual ~ChunkMgr() = default;
    virtual Chunk* get() = 0;
    // Get one chunk where buffer id equals to the given buffer id
    virtual Chunk* get(int buffer_id) = 0;
    // Put one chunk to chunk manager with the given buffer id
    virtual void put(int buffer_id, Chunk* chunk) = 0;
    // TODO: don't need free_size
    virtual int free_size() = 0;
};

/*
 * HPNL provides the default chunk manager implementation, but don't want to prevent HPNL from doing optimizations. User
 * can implement specific buffer manager.
 * The DefaultChunkMgr use stl vector to manage free buffer list and use stl map use build mapping between buffer_id and
 * buffer.
 */
class DefaultChunkMgr : public ChunkMgr {
  public:
    DefaultChunkMgr();
    DefaultChunkMgr(int buffer_num_, uint64_t buffer_size_);
    ~DefaultChunkMgr() override;
    Chunk* get(int id) override;
    void put(int mid, Chunk* ck) override;
    Chunk* get() override;
    int free_size() override;
  protected:
    uint32_t get_id();
  private:
    std::mutex mtx;
    std::vector<Chunk*> bufs;
    std::map<int, Chunk*> buf_map;
    int buffer_num;
    int buffer_size;
    uint32_t buffer_id;
};

class PoolAllocator {
  public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    static char* malloc(size_type bytes);
    static void free(char* block);

    static int buffer_size;
    static int id;
    static std::map<int, Chunk*> chunk_map;
    static std::mutex mtx;
};

class ChunkPool : public boost::pool<PoolAllocator>, public ChunkMgr {
  public:
    ChunkPool(int request_buffer_size,
              int next_request_buffer_number, int max_buffer_number);
    ~ChunkPool() override;
    void *malloc();
    void free(void * const ck);
    Chunk* get(int id) override;
    void put(int mid, Chunk* ck) override;
    Chunk* get() override;
    int free_size() override;
  private:
    void *system_malloc();
  private:
    uint64_t buffer_size;
};

#endif
