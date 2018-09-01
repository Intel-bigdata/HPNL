package com.intel.hpnl.pingpong;

import java.util.Arrays;
import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

public class ConnectedCallback implements Handler {
  public ConnectedCallback() {
    byteArray = new byte[4096];
    Arrays.fill(byteArray, (byte)'0');
    buffer = ByteBuffer.wrap(byteArray);
  }
  public void handle(Connection con, int rdmaBufferId, int blockBufferSize, int blockBufferId, long seq) {
    con.send(buffer, 4096, 0, 0, seq);
  }

  private byte[] byteArray;
  private ByteBuffer buffer;
}
