#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "HPNL/Client.h"
#include "HPNL/HpnlBufMgr.h"

int count = 0;
uint64_t start, end = 0;
std::mutex mtx;

#define BUFFER_SIZE 65536
#define BUFFER_NUM 65536

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
      con->decode_peer_name(ck->buffer, peer_name, 16);
      Chunk *sck = con->encode(ck->buffer, BUFFER_SIZE, peer_name);
      con->send(sck);
    }
  private:
    BufMgr *bufMgr;
};

int main() {
  BufMgr *bufMgr = new HpnlBufMgr(BUFFER_NUM, BUFFER_SIZE);

  Client *client = new Client(1, 16);
  client->init(false);

  client->set_buf_mgr(bufMgr);

  RecvCallback *recvCallback = new RecvCallback(bufMgr);
  client->set_recv_callback(recvCallback);

  client->start();

  Connection *con = client->get_con("127.0.0.1", "12345");
  assert(con);
  char* buffer = (char*)std::malloc(BUFFER_SIZE);
  memset(buffer, '0', BUFFER_SIZE);
  
  char* peer_name = con->get_peer_name();
  auto ck = con->encode(buffer, BUFFER_SIZE, peer_name);
  con->send(ck);

  client->wait();

  delete recvCallback;
  delete client;
  delete bufMgr;

  return 0;
}
