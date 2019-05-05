#ifndef EQHANDLER_H
#define EQHANDLER_H

#include <rdma/fi_cm.h>

#include "HPNL/Callback.h"
#include "demultiplexer/EventHandler.h"

class Proactor;
class FiStack;

class EqHandler : public EventHandler {
  public:
    EqHandler(FiStack *stack_, Proactor *proactor_, fid_eq* eq_) : stack(stack_), proactor(proactor_), eq(eq_), recvCallback(NULL), sendCallback(NULL), acceptRequestCallback(NULL), connectedCallback(NULL), shutdownCallback(NULL) {}
    virtual ~EqHandler() {}
    virtual int handle_event(EventType, void*) override;
    virtual fid_eq* get_handle(void) const override;

    virtual void set_accept_request_callback(Callback*) override;
    virtual void set_connected_callback(Callback*) override;
    virtual void set_shutdown_callback(Callback*) override;
    virtual void set_send_callback(Callback*) override;
    virtual void set_recv_callback(Callback*) override;
    virtual void set_read_callback(Callback*) override;
    
  private:
    FiStack *stack;
    Proactor *proactor;
    fid_eq *eq;
    Callback *recvCallback;
    Callback *sendCallback;
    Callback *readCallback;
    Callback *acceptRequestCallback;
    Callback *connectedCallback;
    Callback *shutdownCallback;
};

#endif
