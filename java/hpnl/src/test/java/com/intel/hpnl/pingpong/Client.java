package com.intel.hpnl.pingpong;

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
    for (int i = 0; i < 4096; i++) {
      byteBufferTmp.put((byte)0);
    }
    byteBufferTmp.flip();

    EqService eqService = new EqService("172.168.2.106", "123456", 1, BUFFER_NUM, false);
    CqService cqService = new CqService(eqService, eqService.getNativeHandle());

    List<Connection> conList = new CopyOnWriteArrayList<Connection>();

    ConnectedCallback connectedCallback = new ConnectedCallback(conList, false);
    ReadCallback readCallback = new ReadCallback(false, eqService);
    ShutdownCallback shutdownCallback = new ShutdownCallback();
    eqService.setConnectedCallback(connectedCallback);
    eqService.setRecvCallback(readCallback);
    eqService.setSendCallback(null);
    eqService.setShutdownCallback(shutdownCallback);

    for (int i = 0; i < BUFFER_NUM*10; i++) {
      ByteBuffer sendBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
      eqService.setSendBuffer(sendBuf, BUFFER_SIZE, i);
    }

    for (int i = 0; i < BUFFER_NUM*2*10; i++) {
      ByteBuffer recvBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
      eqService.setRecvBuffer(recvBuf, BUFFER_SIZE, i);
    }

    cqService.start();
    eqService.start();

    eqService.waitToConnected();
    System.out.println("connected, start to pingpong.");
    
    for (Connection con: conList) {
      RdmaBuffer buffer = con.takeSendBuffer(true);
      buffer.put(byteBufferTmp, (byte)0, 10);
      con.send(buffer.remaining(), buffer.getRdmaBufferId());
    }
    //cqService.shutdown();
    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
