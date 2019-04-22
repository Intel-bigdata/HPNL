package com.intel.hpnl.buffer;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;

import java.nio.ByteBuffer;

public class ReadCallback implements Handler {
  public ReadCallback(boolean is_server, EqService eqService) {
    this.is_server = is_server;
    this.eqService = eqService;
  }
  public synchronized int handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    RdmaBuffer recvBuffer = con.getRecvBuffer(rdmaBufferId);

    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);

    System.out.println(recvByteBuffer.getInt());
    return Handler.RESULT_DEFAULT;
  }
  private int count = 0;
  private long startTime;
  private long endTime;
  private float totally_time = 0;

  boolean is_server = false;
  private EqService eqService;
}
