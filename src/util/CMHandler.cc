#include "CMHandler.h"

int CMHandler::handle_event(EventType et, void *context) {
  fi_eq_cm_entry *entry = (fi_eq_cm_entry*)context;
  if (et == ACCEPT_EVENT) {
    HandlePtr handle = stack->accept(entry->info);
    std::shared_ptr<EventHandler> cmHandler(new CMHandler(stack, reactor, handle));
    if (conntectedCallback) {
      cmHandler->set_conntected_callback(conntectedCallback); 
    }
    if (readCallback) {
      cmHandler->set_read_callback(readCallback); 
    }
    reactor->register_handler(cmHandler);
  } else if (et == CONNECTED_EVENT) {
    HandlePtr cqHandle = ((FIStack*)stack)->connected(entry->fid);
    std::shared_ptr<EventHandler> cqHandler(new CQHandler(stack->get_connection(entry->fid), cqHandle));
    if (readCallback) {
      cqHandler->set_read_callback(readCallback); 
    }
    reactor->register_handler(cqHandler); 
    if (conntectedCallback) {
      (*conntectedCallback)(stack->get_connection(entry->fid), NULL);
    }
  } else if (et == CLOSE_EVENT) {
    auto con = reinterpret_cast<FIConnection*>(stack->get_connection(entry->fid));
    reactor->remove_handler(con->get_cqhandle());
    reactor->remove_handler(get_handle());
    stack->reap(entry->fid);
    con->active = false;
  } else {
  
  }
  return 0;
}

HandlePtr CMHandler::get_handle() const {
  return cmHandle;
}

void CMHandler::set_conntected_callback(Callback *callback) {
  conntectedCallback = callback;
}

void CMHandler::set_shutdown_callback(Callback *callback) {
  shutdownCallback = callback;
}

void CMHandler::set_read_callback(Callback *callback) {
  readCallback = callback;
}
