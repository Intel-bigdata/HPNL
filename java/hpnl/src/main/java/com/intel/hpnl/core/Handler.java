package com.intel.hpnl.core;

/**
 * event callback interface.
 */
public abstract Handler {
  public int handle(Connection con, int rdmaBufferId, int blockBufferSize) {return RESULT_DEFAULT;}
  public int handle(RdmConnection con, int bufferId, int blockBufferSize) {return RESULT_DEFAULT;}

  // result of handle method indicating buffer's status
  public int RESULT_BUF_RELEASED = 0; //buffer has been released already inside handle method.
  public int RESULT_DEFAULT = 1; //buffer need to be released after handle method
}
