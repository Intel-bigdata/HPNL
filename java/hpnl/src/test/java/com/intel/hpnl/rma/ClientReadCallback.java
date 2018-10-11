package com.intel.hpnl.rma;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Buffer;

public class ClientReadCallback implements Handler {
  public ClientReadCallback(Buffer[] buffer) {
    this.buffer = buffer;
  }
  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    System.out.println("client read handler, read sucessfully.");
    for (int i = 0; i < 200; i++) {
      System.out.println("get int " + this.buffer[0].getByteBuffer().getInt());
    }
    this.buffer[0].getByteBuffer().flip();
  }
  private Buffer[] buffer;
}
