package com.intel.hpnl.api;

public interface Handler {
  int RESULT_BUF_RELEASED = 0;
  int RESULT_DEFAULT = 1;

  int handle(Connection connection, int bufferId, int bufferSize);

  default int handle(Connection connection, HpnlBuffer hpnlBuffer){
    throw new UnsupportedOperationException("not implemented yet");
  }
}
