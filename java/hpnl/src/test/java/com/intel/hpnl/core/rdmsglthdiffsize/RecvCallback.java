package com.intel.hpnl.core.rdmsglthdiffsize;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.FrameType;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlBuffer;

import java.nio.ByteBuffer;

public class RecvCallback implements Handler {

  private ByteBuffer peerName;

  public RecvCallback(boolean is_server, int interval, int msgSize, ByteBuffer peerName) {
    this.is_server = is_server;
    this.interval = interval;
    this.msgSize = msgSize;
    this.peerName = peerName;
  }

  public int handle(Connection con, int bufferId, int blockBufferSize) {
    if (!is_server) {
      count++;
      if (count == 1) {
        startTime = System.currentTimeMillis();
        endTime = startTime;
      } else {
        endTime = System.currentTimeMillis();
        if ((total_time = endTime-startTime) >= interval*1000) {
          latency = total_time*1000/(float)count;
          throughput = count*msgSize/1024/1024/(total_time/1000);
          System.out.println(msgSize + " bytes message, latency " + latency + " us");
          System.out.println(msgSize + " bytes message, throughput " + throughput + " MB/s");
          System.out.println("************************************************************");
          count = 0;
        }
      }
    }
    HpnlBuffer recvBuffer = con.getRecvBuffer(bufferId);
    assert(recvBuffer != null);
    ByteBuffer recvByteBuffer = recvBuffer.parse(blockBufferSize);

    HpnlBuffer buffer = con.takeSendBuffer();
    buffer.clear();
    ByteBuffer rawBuffer = buffer.getRawBuffer();
    rawBuffer.position(buffer.getMetadataSize());
    rawBuffer.put(recvByteBuffer);
    int limit = rawBuffer.position();
    buffer.insertMetadata(FrameType.NORMAL.id(), 0L, limit);
    rawBuffer.flip();
    if(is_server){
      con.sendBufferToId(buffer, buffer.remaining(), buffer.getPeerConnectionId());
    }else {
      con.sendBuffer(buffer, buffer.remaining());
    }
//    con.releaseRecvBuffer(bufferId);
    return Handler.RESULT_DEFAULT;
  }

  private long count = 0;
  private long startTime;
  private long endTime;
  private float total_time = 0;
  private float latency = 0;
  private float throughput = 0;

  private boolean is_server = false;
  private int interval;
  private int msgSize;

}
