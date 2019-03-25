package com.intel.hpnl.performance.rdma;

import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.RdmaBuffer;

import java.io.File;

public class Client {
  public static void main(String args[]) {
    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;

    EqService eqService = new EqService(addr, "123456", 1, bufferNbr, false).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();
    RdmaBuffer buffer = eqService.getRmaBuffer(4096*1024);

    ClientConnectedCallback connectedCallback = new ClientConnectedCallback();

    ClientReadCallback readCallback = new ClientReadCallback(new File("received_test.jar"));
    ClientRecvCallback recvCallback = new ClientRecvCallback(false, buffer, readCallback);
    eqService.setConnectedCallback(connectedCallback);
    eqService.setRecvCallback(recvCallback);
    eqService.setSendCallback(null);
    eqService.setReadCallback(readCallback);

    eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

    cqService.start();
    eqService.start();

    eqService.waitToConnected();
    System.out.println("connected, start to remote read.");

    //cqService.shutdown();
    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
