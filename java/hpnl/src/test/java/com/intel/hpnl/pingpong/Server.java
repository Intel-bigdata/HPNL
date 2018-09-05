package com.intel.hpnl.pingpong;

import java.util.ArrayList;
import java.util.List;
import java.nio.ByteBuffer;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.Connection;

public class Server {
  public static void main(String args[]) {
    final int BUFFER_SIZE = 4096*65536;

    EqService eqService = new EqService("172.168.2.106", "123456", true);
    CqService cqService = new CqService(eqService, 1, eqService.getNativeHandle());

    List<Connection> conList = new ArrayList<Connection>();

    ConnectedCallback connectedCallback = new ConnectedCallback(conList, true);
    ReadCallback readCallback = new ReadCallback(true, eqService);
    eqService.setConnectedCallback(connectedCallback);
    eqService.setRecvCallback(readCallback);
    eqService.setSendCallback(null);
    eqService.setShutdownCallback(null);

    ByteBuffer recvBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
    ByteBuffer sendBuf = ByteBuffer.allocateDirect(BUFFER_SIZE);
    assert recvBuf != null;
    eqService.set_recv_buffer(recvBuf, BUFFER_SIZE);
    eqService.set_send_buffer(sendBuf, BUFFER_SIZE);
    
    cqService.start();
    eqService.start(1);
    eqService.join();
  }
}
