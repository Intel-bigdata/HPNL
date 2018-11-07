package com.intel.hpnl.rma;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Buffer;

public class ClientReadCallback implements Handler {
  public ClientReadCallback(Buffer[] buffer) {
    this.buffer = buffer;
  }
  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    System.out.println("client read handler, read sucessfully.");
    ByteBuffer byteBuffer = con.getRmaBuffer(rdmaBufferId);
    System.out.println(byteBuffer.getInt());
  }
  private Buffer[] buffer;
}
