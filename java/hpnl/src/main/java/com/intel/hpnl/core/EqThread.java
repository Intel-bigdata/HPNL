package com.intel.hpnl.core;

import java.util.concurrent.atomic.AtomicBoolean;

public class EqThread extends Thread {
  public EqThread(EqService eqService) {
    this.eqService = eqService;
    running.set(true);
    this.setDaemon(true);
  }

  public void run() {
    while (running.get() || eqService.needReap()) {
      if (this.eqService.wait_eq_event(eqService.getNativeHandle()) == -1) {
        shutdown();
      }
      this.eqService.pendingReap();
    }
    this.eqService.free();
  }
  
  public void shutdown() {
    running.set(false); 
  }

  private EqService eqService;
  private final AtomicBoolean running = new AtomicBoolean(false);
}
