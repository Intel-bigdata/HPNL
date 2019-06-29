#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "HPNL/Server.h"

#include <chrono>
#include <thread>

#include <iostream>

#define BUFFER_SIZE 65536
#define BUFFER_NUM 1024
#define MSG_SIZE 4096

class RecvCallback : public Callback {
  public:
    explicit RecvCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~RecvCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      auto con = (Connection*)ck->con;
      char peer_name[16];
      con->decode_peer_name(ck->buffer, peer_name, 16);
      Chunk *sck = con->encode(ck->buffer, MSG_SIZE, peer_name);
      con->send(sck);
    }
  private:
    ChunkMgr *bufMgr;
};

int main() {
  ChunkMgr *bufMgr = new ChunkPool(BUFFER_SIZE, BUFFER_NUM, BUFFER_NUM*10);

  auto server = new Server(1, 16);
  server->init(false);

  server->set_buf_mgr(bufMgr);

  auto recvCallback = new RecvCallback(bufMgr);
  server->set_recv_callback(recvCallback);

  server->listen("127.0.0.1", "12345");
  server->start();

  server->wait();

  delete recvCallback;
  delete server;
  delete bufMgr;
  return 0;
}
