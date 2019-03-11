package com.intel.hpnl.pingpong;

import java.util.ArrayList;
import java.util.List;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.Connection;

public class Server {
  public static void main(String args[]) {
    String addr = args.length >=1 ? args[0] : "localhost";
    String port = args.length >=2 ? args[1] : "123456";
    int bufferSize = args.length >=3 ? Integer.valueOf(args[2]) : 65536;
    int bufferNbr = args.length >=4 ? Integer.valueOf(args[3]) : 32;
    int workNbr = args.length >=5 ? Integer.valueOf(args[4]) : 3;

    EqService eqService = new EqService(addr, port, workNbr, bufferNbr, true).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();
    
    List<Connection> conList = new ArrayList<Connection>();

    ConnectedCallback connectedCallback = new ConnectedCallback(conList, true);
    ReadCallback readCallback = new ReadCallback(true, eqService);
    eqService.setConnectedCallback(connectedCallback);
    eqService.setRecvCallback(readCallback);

    eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

    eqService.start();
    cqService.start();
    
    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
