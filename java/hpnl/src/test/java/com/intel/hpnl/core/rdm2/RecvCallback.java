package com.intel.hpnl.core.rdm2;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlBuffer;

import java.nio.ByteBuffer;

public class RecvCallback implements Handler {
  private int sleep;
  public RecvCallback(boolean is_server) {
    this(is_server, 5);
  }

  public RecvCallback(boolean is_server, int sleep) {
    this.is_server = is_server;
    this.sleep = sleep;
  }
  public int handle(Connection con, int bufferId, int blockBufferSize) {
    HpnlBuffer recvBuffer = con.getRecvBuffer(bufferId);
    assert(recvBuffer != null);
    ByteBuffer msgBuffer = recvBuffer.parse(blockBufferSize);

    if (!is_server) {
      System.out.println(msgBuffer.getLong());
      return Handler.RESULT_DEFAULT;
    }

    int len = msgBuffer.getInt();
    if(len != msgBuffer.remaining()){
      System.out.println(len + " != "+msgBuffer.remaining());
    }

    try {
      Thread.sleep(sleep);
    }catch (InterruptedException e)
    {}

    con.releaseRecvBuffer(bufferId);

    //read peer name
//    int peerNameLen = msgBuffer.getInt();
//    byte[] peerBytes = new byte[peerNameLen];
//    msgBuffer.get(peerBytes, 0, peerNameLen);
//    ByteBuffer peerName = ByteBuffer.allocateDirect(peerNameLen);
//    peerName.put(peerBytes);
//    peerName.limit(peerName.position());
//    peerName.flip();

//    HpnlBuffer sendBuffer = con.takeSendBuffer();
//    ByteBuffer rawBuffer = sendBuffer.getRawBuffer();
//    rawBuffer.clear();
//    rawBuffer.position(sendBuffer.getMetadataSize());
//    rawBuffer.putLong(123456);
//    int limit = rawBuffer.position();
//    sendBuffer.insertMetadata((byte)0, -1L, limit);
//    rawBuffer.flip();
//
//    con.sendTo(sendBuffer.remaining(), sendBuffer.getBufferId(), peerName);
    return Handler.RESULT_BUF_RELEASED;
  }
  private long count = 0;
  private long startTime;
  private long endTime;
  private float total_time = 0;
  private float latency = 0;
  private float throughput = 0;

  private boolean is_server = false;
}
