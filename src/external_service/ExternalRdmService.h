#ifndef EXTERNALRDMSERVICE_H
#define EXTERNALRDMSERVICE_H

#include <stdint.h>
#include <rdma/fabric.h>
#include <jni.h>

class RdmStack;
class RdmConnection;
class BufMgr;
class Chunk;

class ExternalRdmService {
  public:
    ExternalRdmService(int, int, int, int, int, bool);
    ~ExternalRdmService();
    int init(const char*);
    RdmConnection* listen(const char*, const char*);
    RdmConnection* get_con(const char*, const char*, uint64_t, int, int);
    int wait_event(JNIEnv *, int, int(*process)(JNIEnv *, Chunk *, int, int, int));
    void reap(int64_t);

    void set_recv_buffer(char*, uint64_t, int);
    void set_send_buffer(char*, uint64_t, int);

  private:
    RdmStack *stack;
    fid_cq **cqs;
    int read_batch_size;
    int buffer_num;
    int recv_buffer_num;
    int ctx_num;
    int endpoint_num;
    bool is_server;
    BufMgr *recvBufMgr;
    BufMgr *sendBufMgr;
};

#endif
