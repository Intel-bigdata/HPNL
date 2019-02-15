package com.intel.hpnl.rma;

import java.util.ArrayList;
import java.util.List;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.Connection;

public class Server {
  public static void main(String args[]) {
    final int BUFFER_SIZE = 65536;
    final int BUFFER_NUM = 128;

    EqService eqService = new EqService("172.168.2.106", "123456", 1, BUFFER_NUM, true);
    CqService cqService = new CqService(eqService, eqService.getNativeHandle());

    List<Connection> conList = new ArrayList<Connection>();
    
    ConnectedCallback connectedCallback = new ConnectedCallback(conList, true);
    ServerRecvCallback recvCallback = new ServerRecvCallback(eqService, true);
    eqService.setConnectedCallback(connectedCallback);
    eqService.setRecvCallback(recvCallback);

    eqService.initBufferPool(BUFFER_NUM, BUFFER_SIZE, BUFFER_NUM);

    cqService.start();
    eqService.start();

    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
