#include "HPNL/Connection.h"
#include "HPNL/Server.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/Common.h"
#include "ConBufMgr.h"

#define SIZE 3

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback() {}
    virtual ~ShutdownCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      std::cout << "connection shutdown..." << std::endl;
    }
};

int main(int argc, char *argv[]) {
  BufMgr *recvBufMgr = new ConBufMgr();
  Chunk *ck;
  for (int i = 0; i < MEM_SIZE; i++) {
    ck = new Chunk();
    ck->mid = recvBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    recvBufMgr->add(ck->mid, ck);
  }
  BufMgr *sendBufMgr = new ConBufMgr();
  for (int i = 0; i < MEM_SIZE; i++) {
    ck = new Chunk();
    ck->mid = sendBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    sendBufMgr->add(ck->mid, ck);
  }

  Server *server = new Server("172.168.2.106", "123456");
  server->set_recv_buf_mgr(recvBufMgr);
  server->set_send_buf_mgr(sendBufMgr);

  ShutdownCallback *shutdownCallback = new ShutdownCallback();

  server->set_read_callback(NULL);
  server->set_send_callback(NULL);
  server->set_connected_callback(NULL);
  server->set_shutdown_callback(shutdownCallback);

  server->run(1);

  server->wait();

  delete server;

  int recv_chunk_size = recvBufMgr->get_id();
  assert(recv_chunk_size == MEM_SIZE);
  for (int i = 0; i < recv_chunk_size; i++) {
    Chunk *ck = recvBufMgr->index(i);
    free(ck->buffer);
  }
  int send_chunk_size = sendBufMgr->get_id();
  for (int i = 0; i < send_chunk_size; i++) {
    Chunk *ck = sendBufMgr->index(i);
    free(ck->buffer);
  }

  delete recvBufMgr;
  delete sendBufMgr;

  server = NULL;
  recvBufMgr = NULL;
  sendBufMgr = NULL;

  return 0;
}
