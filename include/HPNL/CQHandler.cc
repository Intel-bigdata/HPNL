#include "CQHandler.h"

CQHandler::CQHandler(Connection *con_, HandlePtr handle_) : con(con_), cqHandle(handle_) {}  

int CQHandler::handle_event(EventType et, void *context) {
  if (et == RECV_EVENT) {
    fi_cq_msg_entry *entry = (fi_cq_msg_entry*)context;
    con->read((char*)entry->op_context, entry->len);
    if (readCallback) {
      (*readCallback)(con, entry); 
    }
  }
  return 0;
}

HandlePtr CQHandler::get_handle() const {
  return cqHandle;
}

void CQHandler::set_read_callback(Callback *callback) {
  readCallback = callback;
}
