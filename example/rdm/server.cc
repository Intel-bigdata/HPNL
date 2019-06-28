#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "HPNL/Server.h"
#include "HPNL/HpnlBufMgr.h"

#include <chrono>
#include <thread>

#define BUFFER_SIZE 65536
#define BUFFER_NUM 65536

class RecvCallback : public Callback {
  public:
    RecvCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~RecvCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      Connection *con = (Connection*)ck->con;
      char peer_name[16];
      con->decode_peer_name(ck->buffer, peer_name, 16);
      Chunk *sck = con->encode(ck->buffer, BUFFER_SIZE, peer_name);
      con->send(sck);
    }
  private:
    BufMgr *bufMgr;
};

int main() {
  BufMgr *bufMgr = new HpnlBufMgr(BUFFER_NUM, BUFFER_SIZE);

  Server *server = new Server(1, 16);
  server->init(false);

  server->set_buf_mgr(bufMgr);

  RecvCallback *recvCallback = new RecvCallback(bufMgr);
  server->set_recv_callback(recvCallback);

  server->listen("127.0.0.1", "12345");
  server->start();

  server->wait();

  delete recvCallback;
  delete server;
  delete bufMgr;
  return 0;
}
