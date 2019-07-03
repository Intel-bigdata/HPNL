#include "HPNL/Connection.h"
#include "HPNL/Server.h"
#include "HPNL/ChunkMgr.h"
#include "HPNL/Callback.h"

#include <iostream>

#define MSG_SIZE 3
#define BUFFER_SIZE 65536
#define BUFFER_NUM 65536
#define MAX_WORKERS 10

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback() = default;
    ~ShutdownCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      std::cout << "connection shutdown..." << std::endl;
    }
};

int main(int argc, char *argv[]) {
  ChunkMgr *bufMgr = new DefaultChunkMgr(BUFFER_NUM, BUFFER_SIZE);

  auto server = new Server(1, 16);
  server->init();
  server->set_buf_mgr(bufMgr);

  auto shutdownCallback = new ShutdownCallback();

  server->set_shutdown_callback(shutdownCallback);

  server->start();
  server->listen("172.168.2.106", "123456");

  server->wait();

  delete server;
  delete bufMgr;
  return 0;
}
