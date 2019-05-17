package com.intel.hpnl.core;

/**
 * event callback interface.
 */
public interface Handler {
  int handle(Connection con, int rdmaBufferId, int blockBufferSize);

  // result of handle method indicating buffer's status
  int RESULT_BUF_RELEASED = 0; //buffer has been released already inside handle method.
  int RESULT_DEFAULT = 1; //buffer need to be released after handle method
}
