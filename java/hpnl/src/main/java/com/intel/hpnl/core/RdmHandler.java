package com.intel.hpnl.core;

public interface RdmHandler {
  public void handle(RdmConnection con, int bufferId, int blockBufferSize);
}
