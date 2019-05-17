package com.intel.hpnl.pingpong;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicInteger;

import com.intel.hpnl.core.*;

public class Server {
  public static void main(String args[]) throws InterruptedException {
    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;
    int workNbr = args.length >=4 ? Integer.valueOf(args[3]) : 4;

    EqService eqService = new EqServerService(workNbr, bufferNbr, bufferSize).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();
    
    List<Connection> conList = new ArrayList<Connection>();

    ConnectedCallback connectedCallback = new ConnectedCallback(conList, eqService,true);
//    ReadCallback readCallback = new ReadCallback(true, eqService);
//    eqService.setConnectedCallback(connectedCallback);
//    eqService.setRecvCallback(readCallback);

    System.out.println(addr);
    ExecutorService executor = Executors.newFixedThreadPool(workNbr + 2);
    eqService.connect(addr, "8077", 0, connectedCallback);
    executor.submit(eqService.getEventTask());
    for(EventTask task : cqService.getEventTasks()){
      executor.submit(task);
    }

    Thread.sleep(10000);

//    cqService.stop();
//    System.out.println("eq stop");
//    eqService.stop();

//    executor.shutdown();
  }
}
