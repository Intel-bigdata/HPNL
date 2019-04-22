package com.intel.hpnl.core;

public interface Handler {
  int handle(Connection con, int rdmaBufferId, int blockBufferSize);

  int RESULT_BUF_RELEASED = 0;
  int RESULT_DEFAULT = 1;
}
