#ifndef EQHANDLER_H
#define EQHANDLER_H

#include <rdma/fi_cm.h>

#include "HPNL/Callback.h"
#include "demultiplexer/EventHandler.h"

class Proactor;
class MsgStack;

class EqHandler : public EventHandler {
  public:
    EqHandler(MsgStack *, Proactor*, fid_eq*);
    ~EqHandler() override = default;
    int handle_event(EventType, void*) override;
    fid_eq* get_handle() const override;

    void set_accept_request_callback(Callback*) override;
    void set_connected_callback(Callback*) override;
    void set_shutdown_callback(Callback*) override;
    void set_send_callback(Callback*) override;
    void set_recv_callback(Callback*) override;
    void set_read_callback(Callback*) override;
    
  private:
    MsgStack *stack;
    Proactor *proactor;
    fid_eq *eq;
    Callback *recvCallback;
    Callback *sendCallback;
    Callback *readCallback{};
    Callback *acceptRequestCallback;
    Callback *connectedCallback;
    Callback *shutdownCallback;
};

#endif
