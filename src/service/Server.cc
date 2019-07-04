#include "HPNL/Server.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Connection.h"
#include "HPNL/Callback.h"
#include "service/Service.h"

Server::Server(int worker_num, int buffer_num) {
  service = new Service(worker_num, buffer_num, true);
}

Server::~Server() {
  delete service;
}

int Server::init(bool msg) {
  return service->init(msg);
}

void Server::start() {
  service->start();
}

int Server::listen(const char* ip_, const char* port_) {
  return service->listen(ip_, port_);
}

void Server::shutdown() {
  service->shutdown();
}

void Server::shutdown(Connection *con) {
  service->shutdown(con);
}

void Server::wait() {
  service->wait();
}

void Server::set_buf_mgr(ChunkMgr* bufMgr) {
  service->set_buf_mgr(bufMgr);
}

void Server::set_send_callback(Callback *callback) {
  service->set_send_callback(callback);
}

void Server::set_recv_callback(Callback *callback) {
  service->set_recv_callback(callback);
}

void Server::set_read_callback(Callback *callback) {
  service->set_recv_callback(callback);
}

void Server::set_connected_callback(Callback *callback) {
  service->set_connected_callback(callback);
}

void Server::set_shutdown_callback(Callback *callback) {
  service->set_shutdown_callback(callback);
}

uint64_t Server::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  return service->reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void Server::unreg_rma_buffer(int buffer_id) {
  service->unreg_rma_buffer(buffer_id);
}

Chunk* Server::get_rma_buffer(int buffer_id) {
  return service->get_rma_buffer(buffer_id);
}
