#include "HPNL/Connection.h"
#include "HPNL/Client.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/Common.h"
#include "PingPongBufMgr.h"

#define SIZE 4096

int count = 0;
uint64_t start, end = 0;
std::mutex mtx;

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
      con->send(buffer, SIZE, SIZE, 0, 0);
      std::free(buffer);
    }
  private:
    BufMgr *bufMgr;
};

class RecvCallback : public Callback {
  public:
    RecvCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~RecvCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      std::lock_guard<std::mutex> lk(mtx);
      count++;
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->index(mid);
      Connection *con = (Connection*)ck->con;
      if (count >= 1000000) {
        con->shutdown();
        end = timestamp_now();
        printf("finished, totally consumes %f s, message round trip time is %f us.\n", (end-start)/1000.0, (end-start)*1000/1000000.0);
        return;
      }
      if (count == 1) {
        printf("start ping-pong.\n");
      }
      if (count == 1) {
        start = timestamp_now(); 
      }
      con->send((char*)ck->buffer, SIZE, SIZE, 0, 0);
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
      Connection *con = (Connection*)ck->con;
      con->take_back_chunk(ck);
    }
  private:
    BufMgr *bufMgr;
};

int main(int argc, char *argv[]) {
  BufMgr *recvBufMgr = new PingPongBufMgr();
  Chunk *ck;
  for (int i = 0; i < MEM_SIZE*2; i++) {
    ck = new Chunk();
    ck->rdma_buffer_id = recvBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    recvBufMgr->add(ck->rdma_buffer_id, ck);
  }
  BufMgr *sendBufMgr = new PingPongBufMgr();
  for (int i = 0; i < MEM_SIZE; i++) {
    ck = new Chunk();
    ck->rdma_buffer_id = sendBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    sendBufMgr->add(ck->rdma_buffer_id, ck);
  }
  Client *client = new Client();
  client->set_recv_buf_mgr(recvBufMgr);
  client->set_send_buf_mgr(sendBufMgr);

  RecvCallback *recvCallback = new RecvCallback(recvBufMgr);
  SendCallback *sendCallback = new SendCallback(sendBufMgr);
  ConnectedCallback *connectedCallback = new ConnectedCallback(sendBufMgr);
  ShutdownCallback *shutdownCallback = new ShutdownCallback(client);

  client->set_recv_callback(recvCallback);
  client->set_send_callback(sendCallback);
  client->set_connected_callback(connectedCallback);
  client->set_shutdown_callback(shutdownCallback);

  client->run("172.168.2.106", "123456", 0, 1, 16);

  client->wait();

  delete shutdownCallback;
  delete connectedCallback;
  delete sendCallback;
  delete recvCallback;
  delete client;

  int recv_chunk_size = recvBufMgr->get_id();
  assert(recv_chunk_size == MEM_SIZE*2);
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

  shutdownCallback = NULL;
  connectedCallback = NULL;
  sendCallback = NULL;
  recvCallback = NULL;
  client = NULL;
  recvBufMgr = NULL;
  sendBufMgr = NULL;

  return 0;
}
