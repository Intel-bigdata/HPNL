package com.intel.hpnl.performance.rdma;

import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.EventTask;
import com.intel.hpnl.core.RdmaBuffer;

import java.io.File;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Client {
  public static void main(String args[]) throws InterruptedException {
    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;

    EqService eqService = new EqService(1, bufferNbr, bufferSize).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();
    RdmaBuffer buffer = eqService.getRmaBuffer(4096*1024);

    ClientConnectedCallback connectedCallback = new ClientConnectedCallback();

    ClientReadCallback readCallback = new ClientReadCallback(new File("received_test.jar"));
    ClientRecvCallback recvCallback = new ClientRecvCallback(false, buffer, readCallback);
//    eqService.setRecvCallback(recvCallback);
//    eqService.setSendCallback(null);
//    eqService.setReadCallback(readCallback);

    ExecutorService executor = Executors.newFixedThreadPool(1);
    eqService.connect(addr, "123456", 0, connectedCallback);
    executor.submit(eqService.getEventTask());
    for(EventTask task : cqService.getEventTasks()){
      executor.submit(task);
    }
    System.out.println("connected.");

    Thread.sleep(1000);

    cqService.stop();
    eqService.stop();

    executor.shutdown();
  }
}
