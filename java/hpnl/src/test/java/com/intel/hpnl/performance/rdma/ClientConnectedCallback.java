package com.intel.hpnl.performance.rdma;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;

import java.nio.ByteBuffer;
import java.util.List;

public class ClientConnectedCallback implements Handler {

  public void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    RdmaBuffer rdmaBuffer = con.takeSendBuffer(true);
    ByteBuffer buffer = ByteBuffer.allocate(8);
    buffer.putLong(0);
    buffer.flip();
    rdmaBuffer.put(buffer, (byte)0, 0);
    con.send(rdmaBuffer.remaining(), rdmaBuffer.getRdmaBufferId());
  }
}
