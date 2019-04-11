package com.intel.hpnl.core;

public interface Handler {
  public void handle(Connection con, int rdmaBufferId, int blockBufferSize);
}
