#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "HPNL/Server.h"
#include "PingPongBufMgr.h"
#include "common.h"

#include <chrono>
#include <thread>

class RecvCallback : public Callback {
  public:
    RecvCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~RecvCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      Connection *con = (Connection*)ck->con;
      char peer_name[16];
      con->decode_peer_name(ck->buffer, peer_name);
      Chunk *sck = con->encode(ck->buffer, SIZE, peer_name);
      con->send(sck);
    }
  private:
    BufMgr *bufMgr;
};

int main() {
  BufMgr *rBufMgr = new PingPongBufMgr();
  for (int i = 0; i < BUFFER_NUM; i++) {
    Chunk *ck = new Chunk();
    ck->buffer_id = rBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    rBufMgr->put(ck->buffer_id, ck);
  }
  BufMgr *sBufMgr = new PingPongBufMgr();
  for (int i = 0; i < BUFFER_NUM; i++) {
    Chunk *ck = new Chunk();
    ck->buffer_id = sBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    sBufMgr->put(ck->buffer_id, ck);
  }

  Server *server = new Server(1, 16);
  server->init(false);

  server->set_recv_buf_mgr(rBufMgr);
  server->set_send_buf_mgr(sBufMgr);

  RecvCallback *recvCallback = new RecvCallback(rBufMgr);
  server->set_recv_callback(recvCallback);

  server->listen("172.168.2.106", "12345");
  server->start();

  server->wait();

  delete recvCallback;
  delete server;

  int recv_chunk_size = rBufMgr->get_id();
  for (int i = 0; i < recv_chunk_size; i++) {
    Chunk *ck = rBufMgr->get(i);
    free(ck->buffer);
  }
  int send_chunk_size = sBufMgr->get_id();
  for (int i = 0; i < send_chunk_size; i++) {
    Chunk *ck = sBufMgr->get(i);
    free(ck->buffer);
  }

  delete rBufMgr;
  delete sBufMgr;

  return 0;
}
