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

    EqService eqService = new EqServerService(workNbr, bufferNbr, bufferSize).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();
    
    List<Connection> conList = new ArrayList<Connection>();

    ServerConnectedCallback connectedCallback = new ServerConnectedCallback(conList, true);
//    ReadCallback readCallback = new ReadCallback(true, eqService);

//    eqService.setRecvCallback(readCallback);

    ExecutorService executor = Executors.newFixedThreadPool(workNbr+1);
    eqService.connect(addr, "123456", 0, connectedCallback);
    executor.submit(eqService.getEventTask());
    for(EventTask task : cqService.getEventTasks()){
      executor.submit(task);
    }

    Thread.sleep(20000);

    System.out.println("1");
    cqService.stop();
    System.out.println("2");
    eqService.stop();
    System.out.println("3");

    executor.shutdown();
    System.out.println("4");
  }
}
