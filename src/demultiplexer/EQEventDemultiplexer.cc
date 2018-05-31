#include "EQEventDemultiplexer.h"

#include <iostream>

EQEventDemultiplexer::EQEventDemultiplexer(LogPtr logger_) {
  logger = logger_;
}

EQEventDemultiplexer::~EQEventDemultiplexer() {}

int EQEventDemultiplexer::wait_event(std::map<HandlePtr, EventHandlerPtr> &eventMap) {
  void *cq_context[MAX_POLL_CNT];
  int ret = 0;
  std::map<HandlePtr, EventHandlerPtr> wait_map;
  for (auto var : eventMap) {
    wait_map.insert(std::make_pair(var.first, var.second));  
  }
  for (auto var : wait_map) {
    HandlePtr handlePtr = var.first;
    logger->log(DEBUG, "EQ EVENT");
    uint32_t event;
    fi_eq_cm_entry entry;
    ret = fi_eq_read((fid_eq*)handlePtr->get_ctx(), &event, &entry, sizeof(entry), 0);
    if (ret == -FI_EAGAIN) {
      continue; 
    } else if (ret < 0) {
      logger->log(CRIT, "EQ ERROR OPERATION");
      fi_eq_err_entry err_entry;
      fi_eq_readerr((fid_eq*)handlePtr->get_ctx(), &err_entry, event);
      continue; 
    } else {
      entry.fid = handlePtr->get_fid();
      if (event == FI_CONNREQ) {
        logger->log(DEBUG, "FI FI_CONNREQ");
        eventMap[handlePtr]->handle_event(ACCEPT_EVENT, &entry); 
      } else if (event == FI_CONNECTED)  {
        logger->log(DEBUG, "FI_CONNECTED");
        eventMap[handlePtr]->handle_event(CONNECTED_EVENT, &entry); 
      } else if (event == FI_SHUTDOWN) {
        logger->log(DEBUG, "FI_SHUTDOWN");
        eventMap[handlePtr]->handle_event(CLOSE_EVENT, &entry); 
      } else {
        logger->log(DEBUG, "DO NOT KNOW OPERATION");
      }
    }
  }
  return 0;
}

int EQEventDemultiplexer::register_event(HandlePtr handle) {
  return 0;
}

int EQEventDemultiplexer::remove_event(HandlePtr handle) {
  return 0;
}
