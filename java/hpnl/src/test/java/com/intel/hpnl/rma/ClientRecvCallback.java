package com.intel.hpnl.rma;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;
import com.intel.hpnl.core.Connection;

public class ClientRecvCallback implements Handler {
  public ClientRecvCallback(boolean is_server, RdmaBuffer buffer) {
    this.is_server = is_server;
    this.buffer = buffer;
  }
  
  public synchronized void handle(final Connection con, int rdmaBufferId, int blockBufferSize) {
    RdmaBuffer recvBuffer = con.getRecvBuffer(rdmaBufferId);
    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);
    if (count++ == 0) {
      System.out.println("client recv.");
    }
    long address = recvByteBuffer.getLong();
    long rkey = recvByteBuffer.getLong();
    con.read(buffer.getRdmaBufferId(), 0, 4096*1024, address, rkey);
  }
  boolean is_server = false;
  private RdmaBuffer buffer;
  private int count = 0;
}
