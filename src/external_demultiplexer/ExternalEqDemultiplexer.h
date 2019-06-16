#ifndef EQEXTERNALMULTIPLEXER_H
#define EQEXTERNALMULTIPLEXER_H

#include <sys/epoll.h>
#include <unistd.h>
#include <rdma/fi_cm.h>

#include <mutex>

#include <unordered_map>

class MsgStack;
class MsgConnection;

class ExternalEqDemultiplexer {
  public:
    ExternalEqDemultiplexer(MsgStack*);
    ~ExternalEqDemultiplexer();
    int init();
    int wait_event(fi_info**, fid_eq**, MsgConnection**);
    int add_event(fid_eq*);
    int delete_event(fid_eq*);
  private:
    MsgStack *stack;
    fid_fabric *fabric;
    struct epoll_event event;
    int epfd;
    std::mutex mtx;

    std::unordered_map<fid*, fid_eq*> fid_map;
};

#endif
