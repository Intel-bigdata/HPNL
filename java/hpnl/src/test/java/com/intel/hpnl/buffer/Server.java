package com.intel.hpnl.buffer;

import com.intel.hpnl.core.*;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Server {
  public static void main(String args[]) throws InterruptedException {
    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;
    int workNbr = args.length >=4 ? Integer.valueOf(args[3]) : 3;

    EqService eqService = new EqServerService(workNbr, bufferNbr, true).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();
    
    List<Connection> conList = new ArrayList<Connection>();

    ServerConnectedCallback connectedCallback = new ServerConnectedCallback(conList, true);
//    ReadCallback readCallback = new ReadCallback(true, eqService);
    eqService.setConnectedCallback(connectedCallback);
//    eqService.setRecvCallback(readCallback);

    eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

    ExecutorService executor = Executors.newFixedThreadPool(workNbr);
    eqService.connect(addr, "123456", 0, 5000);
    executor.submit(eqService.getEventTask());
    for(EventTask task : cqService.getEventTasks()){
      executor.submit(task);
    }

    Thread.sleep(10000);
    
    cqService.stop();
    eqService.stop();

    executor.shutdown();
  }
}
