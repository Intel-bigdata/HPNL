package com.intel.hpnl.pingpong;

import java.nio.ByteBuffer;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.intel.hpnl.core.*;

public class Client {
  public static void main(String args[]) throws InterruptedException {

    ByteBuffer byteBufferTmp = ByteBuffer.allocate(4096);
    for (int i = 0; i < 4096; i++) {
      byteBufferTmp.put((byte)0);
    }
    byteBufferTmp.flip();

    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;

    EqService eqService = new EqService(1, bufferNbr, bufferSize).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();

    List<Connection> conList = new CopyOnWriteArrayList<Connection>();

    ConnectedCallback connectedCallback = new ConnectedCallback(conList, eqService,false);
//    ReadCallback readCallback = new ReadCallback(false, eqService);
//    ShutdownCallback shutdownCallback = new ShutdownCallback();
//    eqService.setConnectedCallback(connectedCallback);
//    eqService.setRecvCallback(readCallback);
//    eqService.setSendCallback(null);
//    eqService.setShutdownCallback(shutdownCallback);

    ExecutorService executor = Executors.newFixedThreadPool(5);
    eqService.connect(addr, "8077", 0, connectedCallback);
    executor.submit(eqService.getEventTask());
    for(EventTask task : cqService.getEventTasks()){
      executor.submit(task);
    }

      Thread.sleep(2000);
    for (Connection con: conList) {
      RdmaBuffer buffer = con.takeSendBuffer(true);
      buffer.put(byteBufferTmp, (byte)0, 10);
      con.send(buffer.remaining(), buffer.getRdmaBufferId());
    }

    Thread.sleep(1000);

    cqService.stop();
    eqService.stop();

    executor.shutdown();
  }
}
