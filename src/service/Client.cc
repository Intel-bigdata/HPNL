#include "HPNL/Client.h"

Client::Client(int worker_num, int buffer_num) : Service(worker_num, buffer_num, false) {} 

int Client::init(bool msg) {
  return Service::init(msg);
}

void Client::start() {
  return Service::start();
}

int Client::connect(const char* ip_, const char* port_) {
  return Service::connect(ip_, port_);
}

void Client::shutdown() {
  Service::shutdown();
}

void Client::shutdown(Connection* con) {
  Service::shutdown(con);
}

void Client::wait() {
  Service::wait();
}

void Client::set_recv_buf_mgr(BufMgr* bufMgr) {
  Service::set_recv_buf_mgr(bufMgr);
}

void Client::set_send_buf_mgr(BufMgr* bufMgr) {
  Service::set_send_buf_mgr(bufMgr);
}

void Client::set_send_callback(Callback *callback) {
  Service::set_send_callback(callback);
}

void Client::set_recv_callback(Callback *callback) {
  Service::set_recv_callback(callback);
}

void Client::set_read_callback(Callback *callback) {
  Service::set_read_callback(callback);
}

void Client::set_connected_callback(Callback *callback) {
  Service::set_connected_callback(callback);
}

uint64_t Client::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  return Service::reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void Client::unreg_rma_buffer(int buffer_id) {
  Service::unreg_rma_buffer(buffer_id);
}

Chunk* Client::get_rma_buffer(int buffer_id) {
  return Service::get_rma_buffer(buffer_id);
}

