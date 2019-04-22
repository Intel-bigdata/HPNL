package com.intel.hpnl.pingpong;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.EventTask;

public class Server {
  public static void main(String args[]) throws InterruptedException {
    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;
    int workNbr = args.length >=4 ? Integer.valueOf(args[3]) : 3;

    EqService eqService = new EqService(workNbr, bufferNbr, true).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();
    
    List<Connection> conList = new ArrayList<Connection>();

    ConnectedCallback connectedCallback = new ConnectedCallback(conList, true);
    ReadCallback readCallback = new ReadCallback(true, eqService);
    eqService.setConnectedCallback(connectedCallback);
//    eqService.setRecvCallback(readCallback);

    eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

    ExecutorService executor = Executors.newFixedThreadPool(workNbr);
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
