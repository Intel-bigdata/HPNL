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
    ExternalRdmService(ExternalRdmService& service) = delete;
    ExternalRdmService& operator=(const ExternalRdmService& service) = delete;

    int init();
    RdmConnection* listen(const char*, const char*);
    RdmConnection* get_con(const char*, const char*);
    int wait_event(Chunk**, int*);

    void set_buffer(char*, uint64_t, int);
  private:
    RdmStack *stack;
    ExternalRdmCqDemultiplexer *demultiplexer;
    int buffer_num;
    bool is_server;
    BufMgr *bufMgr;
};

#endif
