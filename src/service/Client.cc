#include "HPNL/Client.h"

Client::Client() : Service(false) {} 

void Client::run(const char* ip_, const char* port_, int worker_num, int buffer_num) {
  Service::run(ip_, port_, worker_num, buffer_num);
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

void Client::set_recv_callback(Callback *callback) {
  Service::set_recv_callback(callback);
}

void Client::set_connected_callback(Callback *callback) {
  Service::set_connected_callback(callback);
}
