#include "HPNL/EQExternalDemultiplexer.h"

EQExternalDemultiplexer::EQExternalDemultiplexer(FIStack *stack_) : stack(stack_) {}

int EQExternalDemultiplexer::wait_event(fid_eq* eq, fi_info** info) {
  int ret = 0;
  uint32_t event;
  fi_eq_cm_entry entry;
  ret = fi_eq_read(eq, &event, &entry, sizeof(entry), 2000);
  if (ret == -FI_EAGAIN) {
    return 0; 
  } else if (ret < 0) {
    fi_eq_err_entry err_entry;
    fi_eq_readerr(eq, &err_entry, event);
    return -1;
  } else {
    entry.fid = &eq->fid;
    if (event == FI_CONNREQ) {
      *info = entry.info;
      return ACCEPT_EVENT;
    } else if (event == FI_CONNECTED)  {
      return CONNECTED_EVENT;
    } else if (event == FI_SHUTDOWN) {
      auto con = stack->get_connection(entry.fid);
      con->status = DOWN;
      stack->reap(entry.fid);
      return SHUTDOWN;
    } else {
      return 0;
    }
  }
  return 0;
}

