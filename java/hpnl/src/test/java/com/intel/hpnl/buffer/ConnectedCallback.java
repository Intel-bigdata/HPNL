package com.intel.hpnl.buffer;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Handler;

import java.util.List;

public class ConnectedCallback implements Handler {
  public ConnectedCallback(List<Connection> conList, boolean isServer) {
    this.conList = conList;
    this.isServer = isServer;
  }
  public void handle(Connection con, int bufferId, int blockBufferSize) {
    this.conList.add(con);
  }
  List<Connection> conList;
  boolean isServer;
}
