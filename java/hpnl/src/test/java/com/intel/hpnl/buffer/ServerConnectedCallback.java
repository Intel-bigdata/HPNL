package com.intel.hpnl.buffer;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Handler;

import java.nio.ByteBuffer;
import java.util.List;

public class ServerConnectedCallback extends Handler {
  public ServerConnectedCallback(List<Connection> conList, boolean isServer) {
    this.conList = conList;
    this.isServer = isServer;
  }
  public void handle(Connection con, int bufferId, int blockBufferSize) {
    this.conList.add(con);
    for(int i=0; i<50; i++){
      ByteBuffer buffer = ByteBuffer.allocate(20);
      buffer.putInt(5);
      buffer.flip();
      con.send(buffer, (byte)0, i);
    }
  }
  List<Connection> conList;
  boolean isServer;
}
