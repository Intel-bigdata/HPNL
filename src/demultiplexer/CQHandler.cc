#include "CQHandler.h"

CQHandler::CQHandler(Connection *con_, HandlePtr handle_) : con(con_), cqHandle(handle_), readCallback(NULL) {}  

int CQHandler::handle_event(EventType et, void *context) {
  if (et == RECV_EVENT) {
    fi_cq_msg_entry *entry = (fi_cq_msg_entry*)context;
    Chunk *ck = (Chunk*)entry->op_context;
    con->read((char*)ck->buffer, entry->len);
    if (readCallback) {
      (*readCallback)(con, ck->buffer); 
    }
    reinterpret_cast<FIConnection*>(con)->reactivate_chunk(ck);
  } else if (et == SEND_EVENT) {
    fi_cq_msg_entry *entry = (fi_cq_msg_entry*)context;
    Chunk *ck = (Chunk*)entry->op_context;
    std::vector<Chunk*> vec;
    vec.push_back(ck);
    reinterpret_cast<FIConnection*>(con)->send_chunk_to_pool(std::move(vec));
  } else if (et == ERROR_EVENT) {
    fi_cq_err_entry *entry = (fi_cq_err_entry*)context; 
    FIConnection *fi_con = reinterpret_cast<FIConnection*>(con);
    if (fi_con) {
      Chunk *ck = (Chunk*)entry->op_context;
      if (fi_con->active)
        fi_con->reactivate_chunk(ck);
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
