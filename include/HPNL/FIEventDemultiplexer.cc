#include "FIEventDemultiplexer.h"

FIEventDemultiplexer::FIEventDemultiplexer(fid_domain *domain, fid_wait *waitset_, LogPtr logger_) : waitset(waitset_) {
  fi_poll_open(domain, &attr, &pollset);
  logger = logger_;
}

FIEventDemultiplexer::~FIEventDemultiplexer() {
  fi_close(&pollset->fid);
}

int FIEventDemultiplexer::wait_event(std::map<HandlePtr, EventHandlerPtr> &eventMap) {
  void *cq_context[MAX_POLL_CNT];
  int ret = 0;
  ret = fi_wait(waitset, -1);
  if (ret < 0) {
    logger->log(CRIT, "wait set error");
    return -1; 
  }
  ret = fi_poll(pollset, cq_context, MAX_POLL_CNT);
  for (int i = 0; i < ret; ++i) {
    assert(cq_context[i]);
    HandlePtr *handlePtr = (HandlePtr*)cq_context[i];
    HandlePtr handle = *handlePtr; 
    if (handle->get_event() == CQ_EVENT) {
      logger->log(DEBUG, "CQ_EVENT");
      fi_cq_msg_entry entry;
      if (fi_cq_read((fid_cq*)handle->get_ctx(), &entry, 1) == -FI_EAVAIL) {
        fi_cq_err_entry err_entry;
        fi_cq_readerr((fid_cq*)handle->get_ctx(), &err_entry, entry.flags); 
        logger->log(CRIT, "CQ ERROR OPERATION");
        eventMap[handle]->handle_event(ERROR_EVENT, &err_entry);  
        continue;
      }
      if (entry.flags & FI_SEND) {
        logger->log(DEBUG, "SEND OPERATION");
        eventMap[handle]->handle_event(SEND_EVENT, &entry); 
      } else if (entry.flags & FI_RECV) {
        logger->log(DEBUG, "RECV OPERATION");
        eventMap[handle]->handle_event(RECV_EVENT, &entry); 
      } else if (entry.flags & FI_READ) {
        eventMap[handle]->handle_event(READ_EVENT, &entry); 
      } else if (entry.flags & FI_WRITE) {
        eventMap[handle]->handle_event(WRITE_EVENT, &entry); 
      } else {
      
      }
    } else {
      logger->log(DEBUG, "EQ EVENT");
      uint32_t event;
      fi_eq_cm_entry entry;
      if (fi_eq_read((fid_eq*)handle->get_ctx(), &event, &entry, sizeof(entry), 0) < 0) {
        logger->log(CRIT, "EQ ERROR OPERATION");
        fi_eq_err_entry err_entry;
        fi_eq_readerr((fid_eq*)handle->get_ctx(), &err_entry, event);
        continue; 
      }
      entry.fid = handle->get_fid();
      if (event == FI_CONNREQ) {
        logger->log(DEBUG, "FI FI_CONNREQ");
        eventMap[handle]->handle_event(ACCEPT_EVENT, &entry); 
      } else if (event == FI_CONNECTED)  {
        logger->log(DEBUG, "FI_CONNECTED");
        eventMap[handle]->handle_event(CONNECTED_EVENT, &entry); 
      } else if (event == FI_SHUTDOWN) {
        logger->log(DEBUG, "FI_SHUTDOWN");
        eventMap[handle]->handle_event(CLOSE_EVENT, &entry); 
      } else {
        logger->log(DEBUG, "DO NOT KNOW OPERATION");
      }
    }
  }
  return 0;
}

int FIEventDemultiplexer::register_event(HandlePtr handle) {
  assert(pollset);
  fi_poll_add(pollset, handle->get_fid(), 0);
  return 0;
}

int FIEventDemultiplexer::remove_event(HandlePtr handle) {
  fi_poll_del(pollset, handle->get_fid(), 0);
  return 0;
}
