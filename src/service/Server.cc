#include "HPNL/Server.h"

Server::Server(int worker_num, int buffer_num) : Service(worker_num, buffer_num, true) {}

int Server::init(bool msg) {
  return Service::init(msg); 
}

void Server::start() {
  Service::start();
}

int Server::listen(const char* ip_, const char* port_) {
  return Service::listen(ip_, port_);
}

void Server::shutdown() {
  Service::shutdown();
}

void Server::shutdown(Connection *con) {
  Service::shutdown(con);
}

void Server::wait() {
  Service::wait();
}

void Server::set_recv_buf_mgr(BufMgr* bufMgr) {
  Service::set_recv_buf_mgr(bufMgr);
}

void Server::set_send_buf_mgr(BufMgr* bufMgr) {
  Service::set_send_buf_mgr(bufMgr);
}

void Server::set_send_callback(Callback *callback) {
  Service::set_send_callback(callback);
}

void Server::set_recv_callback(Callback *callback) {
  Service::set_recv_callback(callback);
}

void Server::set_read_callback(Callback *callback) {
  Service::set_recv_callback(callback);
}

void Server::set_connected_callback(Callback *callback) {
  Service::set_connected_callback(callback);
}

uint64_t Server::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  return Service::reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void Server::unreg_rma_buffer(int buffer_id) {
  Service::unreg_rma_buffer(buffer_id);
}

Chunk* Server::get_rma_buffer(int buffer_id) {
  return Service::get_rma_buffer(buffer_id);
}

