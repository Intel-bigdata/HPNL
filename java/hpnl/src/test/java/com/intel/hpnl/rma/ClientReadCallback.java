package com.intel.hpnl.rma;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.RdmaBuffer;

public class ClientReadCallback implements Handler {
  public ClientReadCallback() {
  }
  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    ByteBuffer byteBuffer = con.getRmaBuffer(rdmaBufferId);
    if (count == 0) {
      startTime = System.currentTimeMillis();
      System.out.println("allocate memory.");
      for (int i = 0; i < 1024*5; i++) {
        //ByteBuffer buf = ByteBuffer.allocateDirect(4096*1024);
      }
    }

    if (count > 1024*5) {
      endTime = System.currentTimeMillis();
      totally_time = (float)(endTime-startTime)/1000;
      System.out.println("finished, total time is " + totally_time + " s");
      return; 
    }

    ByteBuffer byteBufferTmp = ByteBuffer.allocate(4096);
    byteBufferTmp.putChar('a');
    byteBufferTmp.flip();
    RdmaBuffer sendBuffer = con.takeSendBuffer(true);
    sendBuffer.put(byteBufferTmp, (byte)0, 10);
    con.send(sendBuffer.remaining(), sendBuffer.getRdmaBufferId());
    count++;
  }
  private int count = 0;
  private long startTime;
  private long endTime;
  private float totally_time = 0;
}
