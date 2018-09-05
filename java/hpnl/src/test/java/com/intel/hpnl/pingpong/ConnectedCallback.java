package com.intel.hpnl.pingpong;

import java.util.List;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

public class ConnectedCallback implements Handler {
  public ConnectedCallback(List<Connection> conList, boolean isServer) {
    this.conList = conList;
    this.isServer = isServer;
  }
  public void handle(Connection con, int rdmaBufferId, int blockBufferSize, int blockBufferId, long seq) {
    this.conList.add(con);
  }
  List<Connection> conList;
  boolean isServer;
}
