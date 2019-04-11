package com.intel.hpnl.buffer;

import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.EqService;

import java.nio.ByteBuffer;

public class Client {
  public static void main(String args[]) {

    ByteBuffer byteBufferTmp = ByteBuffer.allocate(4096);
    for (int i = 0; i < 4096; i++) {
      byteBufferTmp.put((byte)0);
    }
    byteBufferTmp.flip();

    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;

    EqService eqService = new EqService(1, bufferNbr, false).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();

    ReadCallback readCallback = new ReadCallback(false, eqService);
    ShutdownCallback shutdownCallback = new ShutdownCallback();
    eqService.setRecvCallback(readCallback);
    eqService.setSendCallback(null);
    eqService.setShutdownCallback(shutdownCallback);

    eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

    cqService.start();
    eqService.connect(addr, "123456", 0);

    System.out.println("connected, start to pingpong.");

    //cqService.shutdown();
    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
