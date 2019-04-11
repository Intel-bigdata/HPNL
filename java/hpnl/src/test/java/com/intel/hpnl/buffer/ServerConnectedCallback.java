package com.intel.hpnl.buffer;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.HpnlBuffer;

import java.nio.ByteBuffer;
import java.util.List;

public class ServerConnectedCallback implements Handler {
  public ServerConnectedCallback(List<Connection> conList, boolean isServer) {
    this.conList = conList;
    this.isServer = isServer;
  }
  public void handle(Connection con, int bufferId, int blockBufferSize) {
    this.conList.add(con);
    for(int i=0; i<50; i++){
      HpnlBuffer hpnlBuffer = con.takeSendBuffer(true);
      ByteBuffer buffer = ByteBuffer.allocate(20);
      buffer.putInt(5);
      buffer.flip();
      hpnlBuffer.put(buffer, (byte)0, i);
      con.send(hpnlBuffer.remaining(), hpnlBuffer.getBufferId());
    }
  }
  List<Connection> conList;
  boolean isServer;
}
