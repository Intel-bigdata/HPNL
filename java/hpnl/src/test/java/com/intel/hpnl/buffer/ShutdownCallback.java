package com.intel.hpnl.buffer;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Handler;

public class ShutdownCallback implements Handler {
  public ShutdownCallback() {
  }
  public void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
  }
}
