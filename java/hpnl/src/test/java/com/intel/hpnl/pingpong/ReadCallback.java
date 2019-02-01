package com.intel.hpnl.pingpong;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;
import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.EqService;

public class ReadCallback implements Handler {
  public ReadCallback(boolean is_server, EqService eqService) {
    this.is_server = is_server;
    this.eqService = eqService;
  }
  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    if (!is_server) {
      if (count == 0) {
        startTime = System.currentTimeMillis();
      }
      if (++count >= 1000000) {
        endTime = System.currentTimeMillis();
        totally_time = (float)(endTime-startTime)/1000;
        System.out.println("finished, total time is " + totally_time + " s");
        return;
      }
    }
    RdmaBuffer sendBuffer = con.takeSendBuffer(true);
    RdmaBuffer recvBuffer = con.getRecvBuffer(rdmaBufferId);

    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);

    sendBuffer.put(recvByteBuffer, (byte)0, 1, 10);
    con.send(sendBuffer.remaining(), sendBuffer.getRdmaBufferId());
  }
  private int count = 0;
  private long startTime;
  private long endTime;
  private float totally_time = 0;

  boolean is_server = false;
  private EqService eqService;
}
