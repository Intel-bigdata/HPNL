#include "HPNL/Connection.h"
#include "HPNL/Server.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/Common.h"
#include "PingPongBufMgr.h"

#define SIZE 3

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback() {}
    virtual ~ShutdownCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      std::cout << "shutdown..." << std::endl;
    }
};

class ReadCallback : public Callback {
  public:
    ReadCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~ReadCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->index(mid);
      Connection *con = (Connection*)ck->con;
      con->write((char*)ck->buffer, SIZE, SIZE);
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
      Chunk *ck = bufMgr->index(mid);
      bufMgr->add(mid, ck);
    }
  private:
    BufMgr *bufMgr;
};

int main(int argc, char *argv[]) {
  BufMgr *recvBufMgr = new PingPongBufMgr();
  Chunk *ck;
  for (int i = 0; i < CON_MEMPOOL_SIZE; i++) {
    ck = new Chunk();
    ck->mid = recvBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    recvBufMgr->add(ck->mid, ck);
  }
  BufMgr *sendBufMgr = new PingPongBufMgr();
  for (int i = 0; i < CON_MEMPOOL_SIZE; i++) {
    ck = new Chunk();
    ck->mid = sendBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    sendBufMgr->add(ck->mid, ck);
  }

  LogPtr logger(new Log("/tmp/log/", "nanolog.server", nanolog::LogLevel::DEBUG));
  logger->start();
  Server *server = new Server("172.168.2.106", "123456", logger);
  server->set_recv_buf_mgr(recvBufMgr);
  server->set_send_buf_mgr(sendBufMgr);

  ReadCallback *readCallback = new ReadCallback(recvBufMgr);
  SendCallback *sendCallback = new SendCallback(sendBufMgr);
  ShutdownCallback *shutdownCallback = new ShutdownCallback();

  server->set_read_callback(readCallback);
  server->set_send_callback(sendCallback);
  server->set_connected_callback(NULL);
  server->set_shutdown_callback(shutdownCallback);

  server->run();

  server->wait();

  delete sendCallback;
  delete readCallback;
  delete server;
  delete recvBufMgr;
  delete sendBufMgr;

  sendCallback = NULL;
  readCallback = NULL;
  server = NULL;
  recvBufMgr = NULL;
  sendBufMgr = NULL;

  return 0;
}
