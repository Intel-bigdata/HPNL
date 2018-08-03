package com.intel.hpnl.core;

public interface Handler {
  public void handle(Connection connection, int blockId);
}
