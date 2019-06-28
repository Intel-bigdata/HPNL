#include "HPNL/Connection.h"
#include "HPNL/Server.h"
#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"
#include "HPNL/HpnlBufMgr.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdint.h>
#include <string.h>
#include <libpmemobj.h>

#include <iostream>

/* size of the pmemobj pool -- 1 GB */
#define POOL_SIZE (1024*1024*1024L)
#define MAX_BUF_LEN 20

/* name of our layout in the pool */
#define LAYOUT_NAME "pmem_spark_shuffle"

#define BUFFER_SIZE 65536
#define BUFFER_NUM 65536

int count = 0;
std::string local_addr, local_rkey, local_len;

struct my_root {
  size_t len;
  char buf[MAX_BUF_LEN];
};

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
    RecvCallback(BufMgr *bufMgr_, Server *server_) : bufMgr(bufMgr_), server(server_) {
      const char path[] = "/dev/dax0.0";
      /* create the pmemobj pool or open it if it already exists */
      pop = pmemobj_open(path, LAYOUT_NAME);
      if (pop == NULL) {
	pop = pmemobj_create(path, LAYOUT_NAME, POOL_SIZE, S_IRUSR | S_IWUSR);
      }

      if (pop == NULL) {
	exit(1);
      }

      PMEMoid root = pmemobj_root(pop, sizeof(struct my_root));
      rootp = (struct my_root*)pmemobj_direct(root);

      struct my_root *data_tmp = (struct my_root*)((uintptr_t)pop+root.off);

      char buf[MAX_BUF_LEN] = "hello world";
      rootp->len = strlen("hello world");

      pmemobj_persist(pop, &rootp->len, sizeof(rootp->len));
      pmemobj_memcpy_persist(pop, rootp->buf, buf, rootp->len);
    }
    virtual ~RecvCallback() {
      pmemobj_close(pop);
    }
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      Connection *con = (Connection*)ck->con;

      if (count == 0) {
	local_addr = std::to_string((long)rootp->buf);
	con->sendBuf(local_addr.c_str(), local_addr.length());
      } else if (count == 1){
	rkey = server->reg_rma_buffer((char*)pop, POOL_SIZE, 0);
	local_rkey = std::to_string(rkey);
	con->sendBuf(local_rkey.c_str(), local_rkey.length());
      } else {
	local_len = std::to_string(rootp->len);
	con->sendBuf(local_len.c_str(), local_len.length());
      }
      count++;
    }
  private:
    BufMgr *bufMgr;
    Server *server;
    PMEMobjpool *pop;
    struct my_root *rootp;
    uint64_t rkey;
};

class SendCallback : public Callback {
  public:
    SendCallback(BufMgr *bufMgr_) : bufMgr(bufMgr_) {}
    virtual ~SendCallback() {}
    virtual void operator()(void *param_1, void *param_2) override {
      int mid = *(int*)param_1;
      Chunk *ck = bufMgr->get(mid);
      Connection *con = (Connection*)ck->con;
      con->activate_send_chunk(ck);
    }
  private:
    BufMgr *bufMgr;
};

int main(int argc, char *argv[]) {
  BufMgr *bufMgr = new HpnlBufMgr(BUFFER_NUM, BUFFER_SIZE);

  Server *server = new Server(1, 16);
  server->init();
  server->set_buf_mgr(bufMgr);

  RecvCallback *recvCallback = new RecvCallback(bufMgr, server);
  SendCallback *sendCallback = new SendCallback(bufMgr);
  ShutdownCallback *shutdownCallback = new ShutdownCallback();

  server->set_recv_callback(recvCallback);
  server->set_send_callback(sendCallback);
  server->set_connected_callback(NULL);
  server->set_shutdown_callback(shutdownCallback);

  server->start();
  server->listen("172.168.0.40", "123456");

  server->wait();

  delete sendCallback;
  delete recvCallback;
  delete server;
  delete bufMgr;

  return 0;
}
