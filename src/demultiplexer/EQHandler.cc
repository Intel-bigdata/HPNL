#include "EQHandler.h"

#include <iostream>

int EQHandler::handle_event(EventType et, void *context) {
  fi_eq_cm_entry *entry = (fi_eq_cm_entry*)context;
  if (et == ACCEPT_EVENT) {
    HandlePtr handle = stack->accept(entry->info);
    std::shared_ptr<EventHandler> eqHandler(new EQHandler(stack, reactor, handle));
    if (connectedCallback) {
      eqHandler->set_connected_callback(connectedCallback); 
    }
    if (readCallback) {
      eqHandler->set_read_callback(readCallback); 
    }
    reactor->register_handler(eqHandler);
  } else if (et == CONNECTED_EVENT) {
    auto con = stack->get_connection(entry->fid);
    if (readCallback) {
      con->set_read_callback(readCallback);
    }
    if (connectedCallback) {
      (*connectedCallback)(stack->get_connection(entry->fid), NULL);
    }
  } else if (et == CLOSE_EVENT) {
    auto con = stack->get_connection(entry->fid);
    reactor->remove_handler(get_handle());
    stack->reap(entry->fid);
    con->active = false;
  } else {
  
  }
  return 0;
}

HandlePtr EQHandler::get_handle() const {
  return eqHandle;
}

void EQHandler::set_connected_callback(Callback *callback) {
  connectedCallback = callback;
}

void EQHandler::set_shutdown_callback(Callback *callback) {
  shutdownCallback = callback;
}


void EQHandler::set_read_callback(Callback *callback) {
  readCallback = callback;
}

Callback* EQHandler::get_read_callback() {
  return readCallback;
}

