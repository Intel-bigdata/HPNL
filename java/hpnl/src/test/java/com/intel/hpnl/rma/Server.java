package com.intel.hpnl.rma;

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
    final int BUFFER_SIZE = 65536;
    final int BUFFER_NUM = 128;

    EqService eqService = new EqService(1, BUFFER_NUM, BUFFER_SIZE).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();

    List<Connection> conList = new ArrayList<Connection>();
    
    ConnectedCallback connectedCallback = new ConnectedCallback(conList, true);
    ServerRecvCallback recvCallback = new ServerRecvCallback(eqService, true);
//    eqService.setRecvCallback(recvCallback);


    ExecutorService executor = Executors.newFixedThreadPool(1);
    eqService.connect("localhost", "123456", 0, connectedCallback);
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
