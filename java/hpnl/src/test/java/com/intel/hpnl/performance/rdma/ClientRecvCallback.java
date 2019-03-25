package com.intel.hpnl.performance.rdma;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;

import java.nio.ByteBuffer;

public class ClientRecvCallback implements Handler {

  private final ClientReadCallback clientReadCallback;

  public ClientRecvCallback(boolean is_server, RdmaBuffer buffer, ClientReadCallback clientReadCallback) {
    this.is_server = is_server;
    this.buffer = buffer;
    this.clientReadCallback = clientReadCallback;
  }
  
  public synchronized void handle(final Connection con, int rdmaBufferId, int blockBufferSize) {
    RdmaBuffer recvBuffer = con.getRecvBuffer(rdmaBufferId);
    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);

    long address = recvByteBuffer.getLong();
    long rkey = recvByteBuffer.getLong();
    long fileLen = recvByteBuffer.getLong();

    if (count == 0) {
      System.out.println("client recv.");
      clientReadCallback.setFileLen(fileLen);
    }

    if(fileLen <= 0){
      return;
    }

    con.read(buffer.getRdmaBufferId(), 0, fileLen, address, rkey);
  }
  boolean is_server = false;
  private RdmaBuffer buffer;
  private int count = 0;
}
