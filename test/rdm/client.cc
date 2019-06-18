#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "HPNL/Client.h"
#include "PingPongBufMgr.h"
#include "common.h"

int count = 0;
uint64_t start, end = 0;
std::mutex mtx;

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

class RecvCallback : public Callback {
  public:
    RecvCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~RecvCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      std::lock_guard<std::mutex> lk(mtx);
      count++;
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      Connection *con = (Connection*)ck->con;
      if (count >= 1000000) {
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
  for (int i = 0; i < BUFFER_NUM*10; i++) {
    Chunk *ck = new Chunk();
    ck->buffer_id = rBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    rBufMgr->put(ck->buffer_id, ck);
  }
  BufMgr *sBufMgr = new PingPongBufMgr();
  for (int i = 0; i < BUFFER_NUM*10; i++) {
    Chunk *ck = new Chunk();
    ck->buffer_id = sBufMgr->get_id();
    ck->buffer = std::malloc(BUFFER_SIZE);
    ck->capacity = BUFFER_SIZE;
    sBufMgr->put(ck->buffer_id, ck);
  }

  Client *client = new Client(1, 16);
  client->init(false);

  client->set_recv_buf_mgr(rBufMgr);
  client->set_send_buf_mgr(sBufMgr);

  RecvCallback *recvCallback = new RecvCallback(rBufMgr);
  client->set_recv_callback(recvCallback);

  client->start();

  Connection *con = client->get_con("172.168.2.106", "12345");
  assert(con);
  char* buffer = (char*)std::malloc(SIZE);
  memset(buffer, '0', SIZE);
  
  char* peer_name = con->get_peer_name();
  Chunk *ck = con->encode(buffer, SIZE, peer_name);
  con->send(ck);

  client->wait();

  delete recvCallback;
  delete client;

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
