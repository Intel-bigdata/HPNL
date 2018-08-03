#ifndef EQEXTERNALMULTIPLEXER_H
#define EQEXTERNALMULTIPLEXER_H

#include "HPNL/FIStack.h"
#include "HPNL/FIConnection.h"

class EQExternalDemultiplexer {
  public:
    EQExternalDemultiplexer(FIStack*);
    int wait_event(fid_eq*, fi_info**);
  private:
    FIStack *stack;
};

#endif
