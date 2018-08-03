#include "HPNL/EQEventDemultiplexer.h"

#include <iostream>

EQEventDemultiplexer::EQEventDemultiplexer(ConMgr *conMgr_, bool is_server_) {
  conMgr = conMgr_;
  is_server = is_server_;
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
    uint32_t event;
    fi_eq_cm_entry entry;
    ret = fi_eq_read((fid_eq*)handlePtr->get_ctx(), &event, &entry, sizeof(entry), 2000);
    if (ret == -FI_EAGAIN) {
      continue; 
    } else if (ret < 0) {
      fi_eq_err_entry err_entry;
      fi_eq_readerr((fid_eq*)handlePtr->get_ctx(), &err_entry, event);
      continue; 
    } else {
      entry.fid = handlePtr->get_fid();
      if (event == FI_CONNREQ) {
        eventMap[handlePtr]->handle_event(ACCEPT_EVENT, &entry); 
      } else if (event == FI_CONNECTED)  {
        conMgr->notify();
        eventMap[handlePtr]->handle_event(CONNECTED_EVENT, &entry); 
      } else if (event == FI_SHUTDOWN) {
        if (!is_server) {
          conMgr->notify();
        }
        eventMap[handlePtr]->handle_event(CLOSE_EVENT, &entry); 
      } else {
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
