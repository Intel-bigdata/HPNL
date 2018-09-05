package com.intel.hpnl.pingpong;

import java.util.Arrays;
import java.nio.ByteBuffer;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.List;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.Connection;

public class Client {
  public static void main(String args[]) {
    final int BUFFER_SIZE = 4096*65536;
    EqService eqService = new EqService("172.168.2.106", "123456", false);
    CqService cqService = new CqService(eqService, 1, eqService.getNativeHandle());

    List<Connection> conList = new CopyOnWriteArrayList<Connection>();

    ConnectedCallback connectedCallback = new ConnectedCallback(conList, false);
    ReadCallback readCallback = new ReadCallback(false, eqService);
    ShutdownCallback shutdownCallback = new ShutdownCallback();
    eqService.setConnectedCallback(connectedCallback);
    eqService.setRecvCallback(readCallback);
    eqService.setSendCallback(null);
    eqService.setShutdownCallback(shutdownCallback);

    ByteBuffer recvBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
    ByteBuffer sendBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
    eqService.set_recv_buffer(recvBuf, BUFFER_SIZE);
    eqService.set_send_buffer(sendBuf, BUFFER_SIZE);

    cqService.start();
    eqService.start(1);

    eqService.waitToConnected();
    System.out.println("connected, start to pingpong.");
    
    for (Connection con: conList) {
      byte[] byteArray = new byte[4096];
      Arrays.fill(byteArray, (byte)'0');
      ByteBuffer buffer = ByteBuffer.wrap(byteArray);
      con.send(buffer, 4096, 0, 0, 0);
    }

    eqService.waitToStop();

    for (Connection con: conList) {
      con.shutdown();
    }

    eqService.join();
    cqService.shutdown();
    cqService.join();
  }
}
