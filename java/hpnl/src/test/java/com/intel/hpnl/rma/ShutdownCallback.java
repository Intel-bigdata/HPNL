package com.intel.hpnl.rma;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

public class ShutdownCallback implements Handler {
  public ShutdownCallback() {
  }
  public int handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    return Handler.RESULT_DEFAULT;
  }
}
