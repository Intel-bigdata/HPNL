package com.intel.hpnl.core;

import java.util.concurrent.atomic.AtomicBoolean;

public class CqThread extends Thread {
  public CqThread(CqService cqService, int index) {
    this.cqService = cqService;
    this.index = index; 
    running.set(true);
    this.setDaemon(true);
  }

  public void run() {
    while (running.get()) {
      if (this.cqService.wait_event(index) == -1) {
        shutdown();
      }
    }
  }

  public void shutdown() {
    running.set(false); 
  }

  private int index;
  private CqService cqService;
  private final AtomicBoolean running = new AtomicBoolean(false);
}
