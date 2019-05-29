#include "HPNL/Connection.h"
#include "HPNL/Server.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "PingPongBufMgr.h"

#define SIZE 4096
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

class RecvCallback : public Callback {
  public:
    RecvCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~RecvCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      Connection *con = (Connection*)ck->con;
      con->send((char*)ck->buffer, SIZE, 0);
    }
  private:
    BufMgr *bufMgr;
};

class SendCallback : public Callback {
  public:
    SendCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~SendCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      Connection *con = (Connection*)ck->con;
      con->reclaim_chunk(ck);
    }
  private:
    BufMgr *bufMgr;
};

int main(int argc, char *argv[]) {
  BufMgr *recvBufMgr = new PingPongBufMgr();
  Chunk *ck;
  for (int i = 0; i < MEM_SIZE*2; i++) {
    ck = new Chunk();
    ck->buffer_id = recvBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    recvBufMgr->put(ck->buffer_id, ck);
  }
  BufMgr *sendBufMgr = new PingPongBufMgr();
  for (int i = 0; i < MEM_SIZE; i++) {
    ck = new Chunk();
    ck->buffer_id = sendBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    sendBufMgr->put(ck->buffer_id, ck);
  }

  Server *server = new Server(1, 16);
  server->init();
  server->set_recv_buf_mgr(recvBufMgr);
  server->set_send_buf_mgr(sendBufMgr);

  RecvCallback *recvCallback = new RecvCallback(recvBufMgr);
  SendCallback *sendCallback = new SendCallback(sendBufMgr);
  ShutdownCallback *shutdownCallback = new ShutdownCallback();

  server->set_recv_callback(recvCallback);
  server->set_send_callback(sendCallback);
  server->set_connected_callback(NULL);
  server->set_shutdown_callback(shutdownCallback);

  server->start();
  server->listen("192.168.2.106", "12345");

  server->wait();

  delete sendCallback;
  delete recvCallback;
  delete server;

  int recv_chunk_size = recvBufMgr->get_id();
  assert(recv_chunk_size == MEM_SIZE*2);
  for (int i = 0; i < recv_chunk_size; i++) {
    Chunk *ck = recvBufMgr->get(i);
    free(ck->buffer);
  }
  int send_chunk_size = sendBufMgr->get_id();
  for (int i = 0; i < send_chunk_size; i++) {
    Chunk *ck = sendBufMgr->get(i);
    free(ck->buffer);
  }

  delete recvBufMgr;
  delete sendBufMgr;

  sendCallback = NULL;
  recvCallback = NULL;
  server = NULL;
  recvBufMgr = NULL;
  sendBufMgr = NULL;

  return 0;
}
