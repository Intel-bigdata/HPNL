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

    ByteBuffer byteBufferTmp = ByteBuffer.allocate(4096);
    for (int i = 0; i < 4096; i++) {
      byteBufferTmp.put((byte)0);
    }
    byteBufferTmp.flip();

    String addr = args.length >=1 ? args[0] : "localhost";
    String port = args.length >=2 ? args[1] : "123456";
    int bufferSize = args.length >=3 ? Integer.valueOf(args[2]) : 65536;
    int bufferNbr = args.length >=4 ? Integer.valueOf(args[3]) : 32;


    EqService eqService = new EqService(addr, port, 1, bufferNbr, false).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();

    List<Connection> conList = new CopyOnWriteArrayList<Connection>();

    ConnectedCallback connectedCallback = new ConnectedCallback(conList, false);
    ReadCallback readCallback = new ReadCallback(false, eqService);
    ShutdownCallback shutdownCallback = new ShutdownCallback();
    eqService.setConnectedCallback(connectedCallback);
    eqService.setRecvCallback(readCallback);
    eqService.setSendCallback(null);
    eqService.setShutdownCallback(shutdownCallback);

    eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

    eqService.start();
    cqService.start();

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
