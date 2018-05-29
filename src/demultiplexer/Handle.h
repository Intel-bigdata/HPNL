#ifndef HANDLE_H
#define HANDLE_H

#include <rdma/fabric.h>

enum Event {
  CQ_EVENT,
  EQ_EVENT
};

class Handle {
  public:
    Handle(fid* id_, Event event_, void* ctx_) : id(id_), event(event_), ctx(ctx_) {}
    fid* get_fid() { return id; }
    Event get_event() { return event; }
    void* get_ctx() { return ctx; }
  private:
    fid *id;
    Event event;
    void *ctx;
};

#endif
