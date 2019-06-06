package com.intel.hpnl.rma;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.HpnlBuffer;
import com.intel.hpnl.core.Connection;

public class ClientRecvCallback extends Handler {
  public ClientRecvCallback(boolean is_server, HpnlBuffer buffer) {
    this.is_server = is_server;
    this.buffer = buffer;
  }
  
  public synchronized void handle(final Connection con, int bufferId, int blockBufferSize) {
    HpnlBuffer recvBuffer = con.getRecvBuffer(bufferId);
    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);
    if (count++ == 0) {
      System.out.println("client recv.");
    }
    long address = recvByteBuffer.getLong();
    long rkey = recvByteBuffer.getLong();
    con.read(buffer.getBufferId(), 0, 4096*1024, address, rkey);
  }
  boolean is_server = false;
  private HpnlBuffer buffer;
  private int count = 0;
}
