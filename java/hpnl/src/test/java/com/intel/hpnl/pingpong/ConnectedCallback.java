package com.intel.hpnl.pingpong;

import java.util.Arrays;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

public class ConnectedCallback implements Handler {
  public ConnectedCallback() {
    charArray = new char[4096];
    Arrays.fill(charArray, '0');
    str = charArray.toString();
  }
  public void handle(Connection connection, int blockId) {
    connection.write(str, 4096, 0);
  }

  private char[] charArray;
  private String str;
}
