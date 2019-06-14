package com.intel.hpnl.rma;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;

public class Server {
  public static void main(String args[]) {
    final int BUFFER_SIZE = 65536;
    final int BUFFER_NUM = 128;

    EqService eqService = new EqService(1, BUFFER_NUM, true).init();
    CqService cqService = new CqService(eqService).init();
    assert(eqService != null);
    assert(cqService != null);
    ServerRecvCallback recvCallback = new ServerRecvCallback(eqService);
    eqService.setRecvCallback(recvCallback);

    eqService.initBufferPool(BUFFER_NUM, BUFFER_SIZE, BUFFER_NUM);

    cqService.start();
    eqService.listen("172.168.2.106", "123456");

    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
