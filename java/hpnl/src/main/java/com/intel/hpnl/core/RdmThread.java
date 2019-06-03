package com.intel.hpnl.core;

import java.util.concurrent.atomic.AtomicBoolean;

public class RdmThread extends Thread {
  public RdmThread(RdmService rdmService) {
    this.rdmService = rdmService;
    running.set(true);
    this.setDaemon(true);
  }

  public void run() {
    while (running.get()) {
      if (this.rdmService.wait_event() == -1) {
        shutdown();
      }
    }
    this.rdmService.free();
  }

  public void shutdown() {
    running.set(false); 
  }

  private RdmService rdmService;
  private final AtomicBoolean running = new AtomicBoolean(false);
}
