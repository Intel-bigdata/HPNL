package com.intel.hpnl.pingpong;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;
import com.intel.hpnl.core.Connection;

public class RecvCallback implements Handler {
  public RecvCallback(boolean is_server, int interval, int msgSize) {
    this.is_server = is_server;
    this.interval = interval;
    this.msgSize = msgSize;
  }
  public void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    if (!is_server) {
      count++;
      if (count == 1) {
        startTime = System.currentTimeMillis();
        endTime = startTime;
      } else {
        endTime = System.currentTimeMillis();
        if ((total_time = endTime-startTime) >= interval*1000) {
          latency = total_time*1000/count;
          throughput = count*msgSize/1024/1024/(total_time/1000);
          System.out.println(msgSize + " bytes message, latency " + latency + " us");
          System.out.println(msgSize + " bytes message, throughput " + throughput + " MB/s");
          System.out.println("************************************************************");
          count = 0;
        }
      }
    }
    RdmaBuffer sendBuffer = con.takeSendBuffer(true);
    RdmaBuffer recvBuffer = con.getRecvBuffer(rdmaBufferId);

    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);

    sendBuffer.put(recvByteBuffer, (byte)0, 10);
    con.send(sendBuffer.remaining(), sendBuffer.getRdmaBufferId());
  }
  private float count = 0;
  private long startTime;
  private long endTime;
  private float total_time = 0;
  private float latency = 0;
  private float throughput = 0;

  private boolean is_server = false;
  private int interval;
  private int msgSize;
}
