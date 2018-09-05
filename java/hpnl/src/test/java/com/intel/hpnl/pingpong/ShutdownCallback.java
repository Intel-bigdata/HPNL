package com.intel.hpnl.pingpong;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

public class ShutdownCallback implements Handler {
  public ShutdownCallback() {
  }
  public void handle(Connection con, int rdmaBufferId, int blockBufferSize, int blockBufferId, long seq) {
  }
}
