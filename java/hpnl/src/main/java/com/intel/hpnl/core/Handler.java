package com.intel.hpnl.core;

public interface Handler {
  public void handle(Connection con, int bufferId, int blockBufferSize);
}
