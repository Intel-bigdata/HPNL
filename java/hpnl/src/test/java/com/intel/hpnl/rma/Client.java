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

    EqService eqService = new EqService("172.168.2.106", "123456", false);
    CqService cqService = new CqService(eqService, 1, eqService.getNativeHandle());
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

    for (int i = 0; i < BUFFER_NUM; i++) {
      ByteBuffer sendBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
      eqService.setSendBuffer(sendBuf, BUFFER_SIZE, i);
    }

    for (int i = 0; i < BUFFER_NUM*2; i++) {
      ByteBuffer recvBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
      eqService.setRecvBuffer(recvBuf, BUFFER_SIZE, i);
    }

    cqService.start();
    eqService.start(1);

    eqService.waitToConnected();
    System.out.println("connected, start to remote read.");
    
    for (Connection con: conList) {
      RdmaBuffer sendBuffer = con.getSendBuffer(true);
      sendBuffer.put(byteBufferTmp, (byte)0, 1, 10);
      con.send(sendBuffer.remaining(), sendBuffer.getRdmaBufferId());
      System.out.println("finished sending.");
    }
    //cqService.shutdown();
    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
