#include "HPNL/Server.h"

Server::Server(const char* ip_, const char* port_, LogPtr logger) : Service(ip_, port_, logger, true) {}

void Server::run() {
  Service::run();
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

void Server::set_read_callback(Callback *callback) {
  Service::set_read_callback(callback);
}

void Server::set_connected_callback(Callback *callback) {
  Service::set_connected_callback(callback);
}
