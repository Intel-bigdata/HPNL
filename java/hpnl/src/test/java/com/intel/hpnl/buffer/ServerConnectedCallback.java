package com.intel.hpnl.buffer;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;

import java.nio.ByteBuffer;
import java.util.List;

public class ServerConnectedCallback implements Handler {
  public ServerConnectedCallback(List<Connection> conList, boolean isServer) {
    this.conList = conList;
    this.isServer = isServer;
  }
  public void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    this.conList.add(con);
    for(int i=0; i<50; i++){
      RdmaBuffer rdmaBuffer = con.takeSendBuffer(true);
      ByteBuffer buffer = ByteBuffer.allocate(20);
      buffer.putInt(5);
      buffer.flip();
      rdmaBuffer.put(buffer, (byte)0, i);
      con.send(rdmaBuffer.remaining(), rdmaBuffer.getRdmaBufferId());
    }
  }
  List<Connection> conList;
  boolean isServer;
}
