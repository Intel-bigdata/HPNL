package com.intel.hpnl.pingpong;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import java.nio.ByteBuffer;

public class Server {
  public static void main(String args[]) {
    final int BUFFER_SIZE = 4096*4;

    EqService eqService = new EqService("172.168.2.106", "123456", true);
    CqService cqService = new CqService(eqService, 1, eqService.getNativeHandle());

    ReadCallback readCallback = new ReadCallback(true);
    eqService.setConnectedCallback(null);
    eqService.setRecvCallback(readCallback);
    eqService.setSendCallback(null);
    eqService.setShutdownCallback(null);

    ByteBuffer recvBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
    ByteBuffer sendBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
    assert recvBuf != null;
    eqService.set_recv_buffer(recvBuf, BUFFER_SIZE);
    eqService.set_send_buffer(sendBuf, BUFFER_SIZE);
    
    cqService.start();
    eqService.start();
    eqService.join();
  }
}
