#include "HPNL/Client.h"

Client::Client(const char *ip_, const char *port_, LogPtr logger) : Service(ip_, port_, logger, false) {} 

void Client::run() {
  Service::run();
}

void Client::shutdown() {
  Service::shutdown();
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

void Client::set_read_callback(Callback *callback) {
  Service::set_read_callback(callback);
}

void Client::set_connected_callback(Callback *callback) {
  Service::set_connected_callback(callback);
}
