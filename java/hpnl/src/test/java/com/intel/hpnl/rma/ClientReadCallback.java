package com.intel.hpnl.rma;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

import java.nio.ByteBuffer;

public class ClientReadCallback implements Handler {
  public ClientReadCallback() {
  }
  public synchronized void handle(Connection con, int bufferId, int blockBufferSize) {
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
    con.send(byteBufferTmp, (byte)0, 10);
    count++;
  }
  private int count = 0;
  private long startTime;
  private long endTime;
  private float totally_time = 0;
}
