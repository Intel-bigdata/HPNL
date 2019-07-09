#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "HPNL/Client.h"

int count = 0;
uint64_t start, end = 0;
std::mutex mtx;

#define BUFFER_SIZE 65536
#define BUFFER_NUM 1024
#define MSG_SIZE 4096

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

class RecvCallback : public Callback {
  public:
    explicit RecvCallback(ChunkMgr *bufMgr_) : bufMgr(bufMgr_) {}
    ~RecvCallback() override = default;
    void operator()(void *param_1, void *param_2) override {
      std::lock_guard<std::mutex> lk(mtx);
      count++;
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      auto con = (Connection*)ck->con;
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
      Chunk *sck = con->encode(ck->buffer, MSG_SIZE, peer_name);
      con->send(sck);
    }
  private:
    ChunkMgr *bufMgr;
};

int main() {

  auto client = new Client(1, 16);
  client->init(false);

  ChunkMgr *bufMgr = new ChunkPool(client, BUFFER_SIZE, BUFFER_NUM, BUFFER_NUM*10);
  client->set_buf_mgr(bufMgr);

  auto recvCallback = new RecvCallback(bufMgr);
  client->set_recv_callback(recvCallback);

  client->start();

  Connection *con = client->get_con("127.0.0.1", "12345");
  assert(con);
  char* buffer = (char*)std::malloc(MSG_SIZE);
  memset(buffer, '0', MSG_SIZE);
  
  char* peer_name = con->get_peer_name();
  auto ck = con->encode(buffer, MSG_SIZE, peer_name);
  assert(ck != nullptr);
  con->send(ck);

  client->wait();

  delete recvCallback;
  delete client;
  delete bufMgr;

  return 0;
}
