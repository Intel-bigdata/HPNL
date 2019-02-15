#include "HPNL/Server.h"

Server::Server(const char* ip_, const char* port_) : Service(ip_, port_, true) {}

void Server::run(int worker_num, int buffer_num) {
  Service::run(worker_num, buffer_num);
}

void Server::shutdown() {
  Service::shutdown();
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

void Server::set_connected_callback(Callback *callback) {
  Service::set_connected_callback(callback);
}
