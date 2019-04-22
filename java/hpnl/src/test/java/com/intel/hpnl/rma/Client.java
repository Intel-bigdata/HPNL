package com.intel.hpnl.rma;

import java.nio.ByteBuffer;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.intel.hpnl.core.*;

public class Client {
  public static void main(String args[]) throws InterruptedException {
    final int BUFFER_SIZE = 65536;
    final int BUFFER_NUM = 32;

    ByteBuffer byteBufferTmp = ByteBuffer.allocate(4096);
    byteBufferTmp.putChar('a');
    byteBufferTmp.flip();

    EqService eqService = new EqService(1, BUFFER_NUM, false).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();
    RdmaBuffer buffer = eqService.getRmaBuffer(4096*1024);

    List<Connection> conList = new CopyOnWriteArrayList<Connection>();

    ConnectedCallback connectedCallback = new ConnectedCallback(conList, false);
    ClientRecvCallback recvCallback = new ClientRecvCallback(false, buffer);
    ClientReadCallback readCallback = new ClientReadCallback();
    ShutdownCallback shutdownCallback = new ShutdownCallback();
    eqService.setConnectedCallback(connectedCallback);
//    eqService.setRecvCallback(recvCallback);
//    eqService.setSendCallback(null);
//    eqService.setReadCallback(readCallback);
//    eqService.setShutdownCallback(shutdownCallback);

    eqService.initBufferPool(BUFFER_NUM, BUFFER_SIZE, BUFFER_NUM);

    ExecutorService executor = Executors.newFixedThreadPool(1);
    eqService.connect("localhost", "123456", 0, 5000);
    executor.submit(eqService.getEventTask());
    for(EventTask task : cqService.getEventTasks()){
      executor.submit(task);
    }

    System.out.println("connected, start to remote read.");
    
    for (Connection con: conList) {
      RdmaBuffer sendBuffer = con.takeSendBuffer(true);
      sendBuffer.put(byteBufferTmp, (byte)0, 10);
      con.send(sendBuffer.remaining(), sendBuffer.getRdmaBufferId());
      System.out.println("finished sending.");
    }

    Thread.sleep(1000);

    cqService.stop();
    eqService.stop();

    executor.shutdown();
  }
}
