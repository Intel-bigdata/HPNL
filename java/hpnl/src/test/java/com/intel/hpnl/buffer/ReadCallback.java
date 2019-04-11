package com.intel.hpnl.buffer;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.HpnlBuffer;

import java.nio.ByteBuffer;

public class ReadCallback implements Handler {
  public ReadCallback() {}
  public synchronized void handle(Connection con, int bufferId, int blockBufferSize) {
    HpnlBuffer recvBuffer = con.getRecvBuffer(bufferId);

    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);

    System.out.println(recvByteBuffer.getInt());
  }
}
