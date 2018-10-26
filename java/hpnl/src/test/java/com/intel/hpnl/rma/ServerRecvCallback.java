package com.intel.hpnl.rma;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Buffer;
import com.intel.hpnl.core.Connection;

public class ServerRecvCallback implements Handler {
  public ServerRecvCallback(boolean is_server, Buffer buffer) {
    this.is_server = is_server;
    this.buffer = buffer;

    this.byteBufferTmp = ByteBuffer.allocate(40960);
    for (int i = 0; i < 200; i++) {
      this.buffer.getRawBuffer().putInt(i*4);
    }

    byteBufferTmp.putLong(this.buffer.getAddress());
    byteBufferTmp.putLong(this.buffer.getRKey());
    byteBufferTmp.flip();
  }
  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    Buffer sendBuffer = con.getSendBuffer(true);

    sendBuffer.put(this.byteBufferTmp, (byte)0, 1, 10);
    con.send(sendBuffer.getRawBuffer().remaining(), sendBuffer.getRdmaBufferId());
  }
  private ByteBuffer byteBufferTmp;

  boolean is_server = false;
  private Buffer buffer;
}
