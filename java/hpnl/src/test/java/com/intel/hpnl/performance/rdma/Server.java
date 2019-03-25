package com.intel.hpnl.performance.rdma;

import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.EqService;

import java.io.File;

public class Server {
  public static void main(String args[]) {

    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;
    String filePath = args.length >=4 ? args[3] : "/home/jiafu/test.jar";

    EqService eqService = new EqService(addr, "123456", 1, bufferNbr, true).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();

    ServerRecvCallback recvCallback = new ServerRecvCallback(eqService, new File(filePath));
    eqService.setRecvCallback(recvCallback);

    eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

    cqService.start();
    eqService.start();

    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
