package com.intel.hpnl.pingpong;

import java.util.Arrays;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

public class ReadCallback implements Handler {
  public ReadCallback(boolean is_server) {
    this.is_server = is_server;
    charArray = new char[4096];
    Arrays.fill(charArray, '0');
    str = charArray.toString();
  }
  public void handle(Connection connection, int blockId) {
    if (!is_server) {
      if (count == 0) {
        startTime = System.currentTimeMillis();
      }
      if (++count >= 1000000) {
        endTime = System.currentTimeMillis();
        totally_time += (float)(endTime-startTime)/1000;
        System.out.println("finished.");
        System.out.println("total time is " + totally_time + " s");
        connection.shutdown();
        return;
      }
    }
    connection.write(str, 4096, 0);
  }
  private int count = 0;
  private long startTime;
  private long endTime;
  private float totally_time = 0;

  private char[] charArray;
  private String str;
  boolean is_server = false;
}
