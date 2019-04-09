package com.intel.hpnl.rma;

import java.nio.ByteBuffer;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.List;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.RdmaBuffer;

public class Client {
  public static void main(String args[]) {
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
    eqService.setRecvCallback(recvCallback);
    eqService.setSendCallback(null);
    eqService.setReadCallback(readCallback);
    eqService.setShutdownCallback(shutdownCallback);

    eqService.initBufferPool(BUFFER_NUM, BUFFER_SIZE, BUFFER_NUM);

    cqService.start();
    eqService.start("172.168.2.106", "123456", 0);

    System.out.println("connected, start to remote read.");
    
    for (Connection con: conList) {
      RdmaBuffer sendBuffer = con.takeSendBuffer(true);
      sendBuffer.put(byteBufferTmp, (byte)0, 10);
      con.send(sendBuffer.remaining(), sendBuffer.getRdmaBufferId());
      System.out.println("finished sending.");
    }
    //cqService.shutdown();
    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
