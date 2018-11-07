package com.intel.hpnl.rma;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Buffer;
import com.intel.hpnl.core.Connection;

public class ServerRecvCallback implements Handler {
  public ServerRecvCallback(boolean is_server, Buffer[] buffer) {
    this.is_server = is_server;
    this.buffer = buffer;

    for (int i = 0; i < 200; i++) {
      this.buffer[i].getRawBuffer().putInt(i*4);
    }
  }
  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    System.out.println("server recv.");
    Buffer sendBuffer = con.getSendBuffer(true);
    ByteBuffer byteBufferTmp = ByteBuffer.allocate(200*16);
    for (int i = 0; i < 200; i++) {
      byteBufferTmp.putLong(this.buffer[i].getAddress());
      byteBufferTmp.putLong(this.buffer[i].getRKey());
    }
    byteBufferTmp.flip();
    sendBuffer.put(byteBufferTmp, (byte)0, 0, 0);
    con.send(sendBuffer.getRawBuffer().remaining(), sendBuffer.getRdmaBufferId());
  }
  boolean is_server = false;
  private Buffer[] buffer;
}
