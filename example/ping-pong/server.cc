#include "HPNL/Connection.h"
#include "HPNL/Server.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/HpnlBufMgr.h"

#define MSG_SIZE 4000
#define BUFFER_SIZE 65536
#define BUFFER_NUM 128

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback() = default;
    ~ShutdownCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      std::cout << "connection shutdown..." << std::endl;
    }
};

class RecvCallback : public Callback {
  public:
    explicit RecvCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~RecvCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      auto ck = bufMgr->get(mid);
      auto con = (Connection*)ck->con;
      con->sendBuf((char*)ck->buffer, MSG_SIZE);
    }
  private:
    BufMgr *bufMgr;
};

class SendCallback : public Callback {
  public:
    explicit SendCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~SendCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      auto con = (Connection*)ck->con;
      con->activate_send_chunk(ck);
    }
  private:
    BufMgr *bufMgr;
};

int main(int argc, char *argv[]) {
  BufMgr *bufMgr = new HpnlBufMgr(BUFFER_NUM, BUFFER_SIZE);

  auto server = new Server(1, 16);
  server->init();
  server->set_buf_mgr(bufMgr);

  auto recvCallback = new RecvCallback(bufMgr);
  auto sendCallback = new SendCallback(bufMgr);
  auto shutdownCallback = new ShutdownCallback();

  server->set_recv_callback(recvCallback);
  server->set_send_callback(sendCallback);
  server->set_connected_callback(nullptr);
  server->set_shutdown_callback(shutdownCallback);

  server->start();
  server->listen("172.168.2.106", "12345");

  server->wait();

  delete sendCallback;
  delete recvCallback;
  delete server;
  delete bufMgr;
  return 0;
}
