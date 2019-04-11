#include <stdlib.h>

#include "HPNL/Connection.h"
#include "HPNL/Client.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/Common.h"
#include "PmemBufMgr.h"

#define SIZE 4096

int count = 0;
long addr, rkey, len;
char rma_buf[4096];

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

class ShutdownCallback : public Callback {
  public:
    ShutdownCallback(Client *_clt) : clt(_clt) {}
    virtual ~ShutdownCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      std::cout << "connection shutdown..." << std::endl;
      clt->shutdown();
    }
  private:
    Client *clt;
};

class ConnectedCallback : public Callback {
  public:
    ConnectedCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~ConnectedCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      Connection *con = (Connection*)param_1;
      char* buffer = (char*)std::malloc(SIZE);
      memset(buffer, '0', SIZE);
      con->send(buffer, SIZE, 0);
      std::free(buffer);
    }
  private:
    BufMgr *bufMgr;
};

class RecvCallback : public Callback {
  public:
    RecvCallback(BufMgr *bufMgr_, Client *client_) : bufMgr(bufMgr_), client(client_) {}
    virtual ~RecvCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);

      char* buffer = (char*)std::malloc(SIZE);
      memset(buffer, '0', SIZE);
      Connection *con = (Connection*)ck->con;

      if (count == 0) {
        char* buf = (char*)ck->buffer;
        addr = atol(buf);
        con->send(buffer, SIZE, 0);
      } else if (count == 1){
        char* buf = (char*)ck->buffer;
        rkey = atol(buf);
        con->send(buffer, SIZE, 0);
      } else {
        char* buf = (char*)ck->buffer;
        len = atol(buf);
        client->reg_rma_buffer(rma_buf, 4096, 0);
        con->read(0, 0, len, addr, rkey);
      }
      count++;
    }
  private:
    BufMgr *bufMgr;
    Client *client;
};

class SendCallback : public Callback {
  public:
    SendCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~SendCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      Connection *con = (Connection*)ck->con;
      con->take_back_chunk(ck);
    }
  private:
    BufMgr *bufMgr;
};

class ReadCallback : public Callback {
  public:
    ReadCallback() {}
    virtual ~ReadCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      std::cout << "rma buffer " << rma_buf << std::endl;
    }
};

int main(int argc, char *argv[]) {
  BufMgr *recvBufMgr = new PmemBufMgr();
  Chunk *ck;
  for (int i = 0; i < MEM_SIZE*2; i++) {
    ck = new Chunk();
    ck->buffer_id = recvBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    recvBufMgr->put(ck->buffer_id, ck);
  }
  BufMgr *sendBufMgr = new PmemBufMgr();
  for (int i = 0; i < MEM_SIZE; i++) {
    ck = new Chunk();
    ck->buffer_id = sendBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    sendBufMgr->put(ck->buffer_id, ck);
  }
  Client *client = new Client();
  client->set_recv_buf_mgr(recvBufMgr);
  client->set_send_buf_mgr(sendBufMgr);

  RecvCallback *recvCallback = new RecvCallback(recvBufMgr, client);
  SendCallback *sendCallback = new SendCallback(sendBufMgr);
  ReadCallback *readCallback = new ReadCallback();
  ConnectedCallback *connectedCallback = new ConnectedCallback(sendBufMgr);
  ShutdownCallback *shutdownCallback = new ShutdownCallback(client);

  client->set_recv_callback(recvCallback);
  client->set_send_callback(sendCallback);
  client->set_read_callback(readCallback);
  client->set_connected_callback(connectedCallback);
  client->set_shutdown_callback(shutdownCallback);

  client->run("172.168.0.40", "123456", 1, 16);

  client->wait();

  delete shutdownCallback;
  delete connectedCallback;
  delete sendCallback;
  delete recvCallback;
  delete client;

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

  shutdownCallback = NULL;
  connectedCallback = NULL;
  sendCallback = NULL;
  recvCallback = NULL;
  client = NULL;
  recvBufMgr = NULL;
  sendBufMgr = NULL;

  return 0;
}
