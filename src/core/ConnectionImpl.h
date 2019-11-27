#ifndef CPPCONNECTION_H
#define CPPCONNECTION_H

#include <mutex>

#include <iostream>
#include "HPNL/Connection.h"

class ConnectionImpl : public Connection {
 public:
  virtual ~ConnectionImpl() = default;

  int init() override = 0;
  int shutdown() override = 0;

  int send(Chunk* ck) override = 0;
  int read(Chunk* ck, int local_buffer_offset, uint64_t local_buffer_length,
           uint64_t remote_buffer_address, uint64_t remote_buffer_rkey) override = 0;
  int write(Chunk* ck, int local_buffer_offset, uint64_t local_buffer_length,
           uint64_t remote_buffer_address, uint64_t remote_buffer_rkey) override = 0;

  char* get_peer_name() override { return nullptr; }  /// only uesd by RDM
  void encode_(Chunk* ck, void* buffer, int buffer_length, char* peer_name) {}  /// only used by RDM
  void decode_(Chunk* ck, void* buffer, int* buffer_length, char* peer_name) {}  /// only used by RDM

  void log_used_chunk(Chunk* ck) override { used_chunks[ck->buffer_id] = (Chunk*)ck; }
  void remove_used_chunk(Chunk* ck) override { used_chunks.erase(ck->buffer_id); }

  virtual int activate_recv_chunk(Chunk* ck) = 0;
  virtual void set_recv_callback(Callback*) = 0;
  virtual void set_send_callback(Callback*) = 0;
  virtual Callback* get_recv_callback() = 0;
  virtual Callback* get_send_callback() = 0;

  /// only used by RDM by java binding
  virtual int send(int buffer_size, int id) { return -1; }
  virtual int sendTo(int buffer_size, int buffer_id, const char* peer_address) { return -1; }
  virtual int sendBuf(const char* buffer, int buffer_size) { return -1; }
  virtual int sendBufTo(const char* buffer, int buffer_size, const char* peer_address) { return -1; }
  virtual fi_addr_t recv(const char* buffer, int buffer_id) { return -1; }
  virtual int read(int local_buffer_id, int local_buffer_offset, uint64_t local_buffer_length,
           uint64_t remote_buffer_address, uint64_t remote_buffer_rkey) {
    return -1;
  }
 protected:
  std::map<int, Chunk*> used_chunks;
};

#endif
