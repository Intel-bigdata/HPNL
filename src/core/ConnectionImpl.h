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
  virtual int send(int buffer_size, int id) { return 0; }
  virtual int sendTo(int buffer_size, int buffer_id, const char* peer_address) {
    return 0;
  }
  virtual int sendBuf(const char* buffer, int buffer_size) { return 0; };
  virtual int sendBufTo(const char* buffer, int buffer_size, const char* peer_address) {
    return 0;
  }
  virtual fi_addr_t recv(const char* buffer, int buffer_id) { return 0; }
  int read(int local_buffer_id, int local_buffer_offset, uint64_t local_buffer_length,
           uint64_t remote_buffer_address, uint64_t remote_buffer_rkey) override {
    return 0;
  }

  char* get_peer_name() override { return nullptr; }
  void encode_(Chunk* ck, void* buffer, int buffer_length, char* peer_name) override {}
  void decode_(Chunk* ck, void* buffer, int* buffer_length, char* peer_name) override {}

  virtual int activate_recv_chunk(Chunk* ck) { return 0; }

  void log_used_chunk(Chunk* ck) override = 0;
  void remove_used_chunk(Chunk* ck) override{};

  virtual void set_recv_callback(Callback*) = 0;
  virtual void set_send_callback(Callback*) = 0;
  virtual Callback* get_recv_callback() = 0;
  virtual Callback* get_send_callback() = 0;
};

#endif
