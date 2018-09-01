package com.intel.hpnl.pingpong;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;

public class ShutdownCallback implements Handler {
  public ShutdownCallback(EqService eqService, CqService cqService) {
    this.eqService = eqService;
    this.cqService = cqService;
  }
  public void handle(Connection con, int rdmaBufferId, int blockBufferSize, int blockBufferId, long seq) {
    cqService.shutdown();
    cqService.join();
    eqService.shutdown();
  }

  private EqService eqService;
  private CqService cqService;
}
