package com.intel.hpnl.rma;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Buffer;
import com.intel.hpnl.core.Connection;

public class ClientRecvCallback implements Handler {
  public ClientRecvCallback(boolean is_server, Buffer[] buffer) {
    this.is_server = is_server;
    this.buffer = buffer;
  }
  public synchronized void handle(final Connection con, int rdmaBufferId, int blockBufferSize) {
    System.out.println("block buffer size " + blockBufferSize);
    Buffer recvBuffer = con.getRecvBuffer(rdmaBufferId);
    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);

    for (int i = 0; i < 200; i++) {
      long address = recvByteBuffer.getLong();
      long rkey = recvByteBuffer.getLong();
      con.read(buffer[i].getRdmaBufferId(), 0, 4, address, rkey);
    }
  }
  boolean is_server = false;
  private Buffer[] buffer;
}
