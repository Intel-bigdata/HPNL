package com.intel.hpnl.performance.rdma;

import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.EventTask;

import java.io.File;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Server {
  public static void main(String args[]) throws InterruptedException {

    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;
    String filePath = args.length >=4 ? args[3] : "/home/jiafu/test.jar";

    EqService eqService = new EqService(1, bufferNbr, true).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();

    ServerRecvCallback recvCallback = new ServerRecvCallback(eqService, new File(filePath));
//    eqService.setRecvCallback(recvCallback);

    eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

    ExecutorService executor = Executors.newFixedThreadPool(1);
    eqService.connect(addr, "123456", 0, 5000);
    executor.submit(eqService.getEventTask());
    for(EventTask task : cqService.getEventTasks()){
      executor.submit(task);
    }

    Thread.sleep(1000);

    cqService.stop();
    eqService.stop();

    executor.shutdown();
  }
}
