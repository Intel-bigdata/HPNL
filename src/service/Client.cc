#include "HPNL/Client.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Connection.h"
#include "HPNL/Callback.h"
#include "service/Service.h"

Client::Client(int worker_num, int buffer_num) {
  service = new Service(worker_num, buffer_num, false);
}

Client::~Client() {
  delete service;
}

int Client::init(bool msg) {
  return service->init(msg);
}

void Client::start() {
  return service->start();
}

int Client::connect(const char* ip_, const char* port_) {
  return service->connect(ip_, port_);
}

void Client::shutdown() {
  service->shutdown();
}

void Client::shutdown(Connection* con) {
  service->shutdown(con);
}

Connection* Client::get_con(const char *addr, const char *port) {
  return service->get_con(addr, port);
}

void Client::wait() {
  service->wait();
}

void Client::set_buf_mgr(ChunkMgr* bufMgr) {
  service->set_buf_mgr(bufMgr);
}

void Client::set_send_callback(Callback *callback) {
  service->set_send_callback(callback);
}

void Client::set_recv_callback(Callback *callback) {
  service->set_recv_callback(callback);
}

void Client::set_read_callback(Callback *callback) {
  service->set_read_callback(callback);
}

void Client::set_connected_callback(Callback *callback) {
  service->set_connected_callback(callback);
}

void Client::set_shutdown_callback(Callback *callback) {
  service->set_shutdown_callback(callback);
}

uint64_t Client::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  return service->reg_rma_buffer(buffer, buffer_size, buffer_id);
}

void Client::unreg_rma_buffer(int buffer_id) {
  service->unreg_rma_buffer(buffer_id);
}
