package com.intel.hpnl.pingpong;

import java.util.List;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;

public class ConnectedCallback implements Handler {
  public ConnectedCallback(List<Connection> conList, EqService eqService, boolean isServer) {
    this.conList = conList;
    this.eqService = eqService;
    this.isServer = isServer;
  }
  public int handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    this.conList.add(con);
    System.out.println("connected, start to pingpong.");
    con.setRecvCallback(new ReadCallback(isServer, eqService));
    return Handler.RESULT_DEFAULT;
  }
  List<Connection> conList;
  EqService eqService;
  boolean isServer;
}
