#include "HPNL/Connection.h"
#include "HPNL/Client.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/HpnlBufMgr.h"

#define MSG_SIZE 4096
#define BUFFER_SIZE 65536
#define BUFFER_NUM 65536
#define MAX_WORKERS 10

uint64_t start, end = 0;

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback(Client *_clt) : clt(_clt) {}
    virtual ~ShutdownCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      clt->shutdown();
    }
  private:
    Client *clt;
};

class ConnectedCallback : public Callback {
  public:
    ConnectedCallback(Client *client_, BufMgr *bufMgr_) : client(client_), bufMgr(bufMgr_) {}
    virtual ~ConnectedCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      Connection *con = (Connection*)param_1;
      client->shutdown(con);
    }
  private:
    Client *client;
    BufMgr *bufMgr;
};

void connect() {
  BufMgr *bufMgr = new HpnlBufMgr(BUFFER_NUM, BUFFER_SIZE);

  Client *client = new Client(1, 16);
  client->init();
  client->set_buf_mgr(bufMgr);

  ConnectedCallback *connectedCallback = new ConnectedCallback(client, bufMgr);
  ShutdownCallback *shutdownCallback = new ShutdownCallback(client);

  client->set_recv_callback(NULL);
  client->set_send_callback(NULL);
  client->set_connected_callback(connectedCallback);
  client->set_shutdown_callback(shutdownCallback);

  client->start();
  for (int i = 0; i < 1; i++) {
    client->connect("172.168.2.106", "123456");
  }

  client->wait();

  delete shutdownCallback;
  delete connectedCallback;
  delete client;
  delete bufMgr;
}

int main(int argc, char *argv[]) {
  connect();  
  return 0;
}
