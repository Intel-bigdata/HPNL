#include "HPNL/Connection.h"
#include "HPNL/Server.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/HpnlBufMgr.h"

#define SIZE 3
#define BUFFER_SIZE 65536
#define MEM_SIZE 65536
#define MAX_WORKERS 10

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback() {}
    virtual ~ShutdownCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      std::cout << "connection shutdown..." << std::endl;
    }
};

int main(int argc, char *argv[]) {
  BufMgr *bufMgr = new HpnlBufMgr();
  Chunk *ck;
  for (int i = 0; i < MEM_SIZE*2; i++) {
    ck = new Chunk();
    ck->buffer_id = bufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    bufMgr->put(ck->buffer_id, ck);
  }

  Server *server = new Server(1, 16);
  server->init();
  server->set_buf_mgr(bufMgr);

  ShutdownCallback *shutdownCallback = new ShutdownCallback();

  server->set_recv_callback(NULL);
  server->set_send_callback(NULL);
  server->set_connected_callback(NULL);
  server->set_shutdown_callback(shutdownCallback);

  server->start();
  server->listen("172.168.2.106", "123456");

  server->wait();

  delete server;

  for (int i = 0; i < MEM_SIZE*2; i++) {
    Chunk *ck = bufMgr->get(i);
    free(ck->buffer);
  }
  delete bufMgr;

  server = NULL;
  bufMgr = NULL;

  return 0;
}
