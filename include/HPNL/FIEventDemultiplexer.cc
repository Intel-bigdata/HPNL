#include "FIEventDemultiplexer.h"
#include <assert.h>

FIEventDemultiplexer::FIEventDemultiplexer(fid_domain *domain) {
  fi_poll_open(domain, &attr, &pollset);
}

FIEventDemultiplexer::~FIEventDemultiplexer() {
  fi_close(&pollset->fid);
}

int FIEventDemultiplexer::wait_event(std::map<HandlePtr, EventHandlerPtr> &eventMap) {
  void *cq_context[MAX_POLL_CNT];
  int ret = fi_poll(pollset, cq_context, MAX_POLL_CNT);
  for (int i = 0; i < ret; ++i) {
    printf("get %d poll.\n", ret);
    assert(cq_context[i]);
    HandlePtr *handlePtr = (HandlePtr*)cq_context[i];
    HandlePtr handle = *handlePtr; 
    if (handle->get_event() == CQ_EVENT) {
      printf("CQ_EVENT.\n");
      fi_cq_msg_entry entry;
      fi_cq_read((fid_cq*)handle->get_ctx(), &entry, 1);
      if (entry.flags & FI_SEND) {
        printf("send event.\n");
        eventMap[handle]->handle_event(SEND_EVENT, &entry); 
      } else if (entry.flags & FI_RECV) {
        eventMap[handle]->handle_event(RECV_EVENT, &entry); 
        printf("recv event.\n");
      } else if (entry.flags & FI_READ) {
        eventMap[handle]->handle_event(READ_EVENT, &entry); 
      } else if (entry.flags & FI_WRITE) {
        eventMap[handle]->handle_event(WRITE_EVENT, &entry); 
      } else {
      
      }
    } else {
      printf("EQ_EVENT.\n");
      uint32_t event;
      fi_eq_cm_entry entry;
      fi_eq_read((fid_eq*)handle->get_ctx(), &event, &entry, sizeof(entry), 0);
      entry.fid = handle->get_fid();
      if (event == FI_CONNREQ) {
        eventMap[handle]->handle_event(ACCEPT_EVENT, &entry); 
      } else if (event == FI_CONNECTED)  {
        eventMap[handle]->handle_event(CONNECTED_EVENT, &entry); 
      } else if (event == FI_SHUTDOWN) {
        eventMap[handle]->handle_event(CLOSE_EVENT, &entry); 
      } else {
      
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
