#include "core/MsgStack.h"
#include "core/MsgConnection.h"
#include "demultiplexer/EqHandler.h"
#include "demultiplexer/Proactor.h"

int EqHandler::handle_event(EventType et, void *context) {
  fi_eq_cm_entry *entry = (fi_eq_cm_entry*)context;
  if (et == ACCEPT_EVENT) {
    assert(acceptRequestCallback);
    BufMgr *recv_buf_mgr;
    BufMgr *send_buf_mgr;
    (*acceptRequestCallback)(&recv_buf_mgr, &send_buf_mgr);

    fid_eq *eq = stack->accept(entry->info, recv_buf_mgr, send_buf_mgr);
    std::shared_ptr<EqHandler> eqHandler = std::make_shared<EqHandler>(stack, proactor, eq);
    if (connectedCallback) {
      eqHandler->set_connected_callback(connectedCallback); 
    }
    if (recvCallback) {
      eqHandler->set_recv_callback(recvCallback); 
    }
    if (sendCallback) {
      eqHandler->set_send_callback(sendCallback);
    }
    if (readCallback) {
      eqHandler->set_read_callback(readCallback);
    }
    if (shutdownCallback) {
      eqHandler->set_shutdown_callback(shutdownCallback);
    }
    proactor->register_handler(eqHandler);
  } else if (et == CONNECTED_EVENT) {
    auto con = stack->get_connection(entry->fid);
    assert(con);
    if (recvCallback) {
      con->set_recv_callback(recvCallback);
    }
    if (shutdownCallback) {
      con->set_shutdown_callback(shutdownCallback);
    }
    if (sendCallback) {
      con->set_send_callback(sendCallback);
    }
    if (readCallback) {
      con->set_read_callback(readCallback);
    }
   
    {
      std::lock_guard<std::mutex> l(con->con_mtx);
      con->status = CONNECTED;
    }
    con->con_cv.notify_one();

    con->init_addr();

    if (connectedCallback) {
      (*connectedCallback)(con, NULL);
    }
  } else if (et == CLOSE_EVENT) {
    auto con = stack->get_connection(entry->fid);
    con->status = SHUTDOWN_REQ;
    if (con->get_shutdown_callback()) {
      (*(con->get_shutdown_callback()))(NULL, NULL);
    }
    proactor->remove_handler(&get_handle()->fid);
    con->status = DOWN;
    stack->reap(entry->fid);
  } else {
    // TODO: exception handler
  }
  return 0;
}

fid_eq* EqHandler::get_handle() const {
  return eq;
}

void EqHandler::set_accept_request_callback(Callback *callback) {
  acceptRequestCallback = callback;
}

void EqHandler::set_connected_callback(Callback *callback) {
  connectedCallback = callback;
}

void EqHandler::set_shutdown_callback(Callback *callback) {
  shutdownCallback = callback;
}

void EqHandler::set_send_callback(Callback *callback) {
  sendCallback = callback;
}

void EqHandler::set_recv_callback(Callback *callback) {
  recvCallback = callback;
}

void EqHandler::set_read_callback(Callback *callback) {
  readCallback = callback;
}

