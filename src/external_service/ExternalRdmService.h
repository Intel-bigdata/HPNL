#ifndef EXTERNALRDMSERVICE_H
#define EXTERNALRDMSERVICE_H

#include <stdint.h>

class RdmStack;
class RdmConnection;
class BufMgr;
class Chunk;
class ExternalRdmCqDemultiplexer;

class ExternalRdmService {
  public:
    ExternalRdmService(int, bool);
    ~ExternalRdmService();
    int init(const char*);
    RdmConnection* listen(const char*, const char*);
    RdmConnection* get_con(const char*, const char*);
    int wait_event(Chunk**, int*);
    void reap(int64_t);

    void set_recv_buffer(char*, uint64_t, int);
    void set_send_buffer(char*, uint64_t, int);

  private:
    RdmStack *stack;
    ExternalRdmCqDemultiplexer *demulti_plexer;
    int buffer_num;
    bool is_server;
    BufMgr *recvBufMgr;
    BufMgr *sendBufMgr;
};

#endif
