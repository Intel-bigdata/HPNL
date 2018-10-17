package com.intel.hpnl.rma;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Buffer;
import com.intel.hpnl.core.Connection;

public class ClientRecvCallback implements Handler {
  public ClientRecvCallback(boolean is_server, Buffer[] buffer) {
    this.is_server = is_server;
    this.buffer = buffer;
  }
  public synchronized void handle(final Connection con, int rdmaBufferId, int blockBufferSize) {
    Buffer recvBuffer = con.getRecvBuffer(rdmaBufferId);
    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);

    final Buffer[] buf = this.buffer;
    final long address = recvByteBuffer.getLong();
    final long rkey = recvByteBuffer.getLong();

    for (int i = 0; i < 200; i++) {
      final int offset = i*4;
      new Thread(new Runnable() {
        public void run() {
          con.read(buf[0].getRdmaBufferId(), offset, 4, address+offset, rkey);
        }
      }).start();
    }
  }
  boolean is_server = false;
  private Buffer[] buffer;
}
