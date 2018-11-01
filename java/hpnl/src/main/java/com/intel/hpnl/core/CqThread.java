package com.intel.hpnl.core;

import java.util.concurrent.atomic.AtomicBoolean;

public class CqThread extends Thread {
  public CqThread(CqService cqService, int index, long affinity) {
    this.cqService = cqService;
    this.index = index; 
    this.affinity = affinity;
    running.set(true);
  }

  public void run() {
    Utils.setAffinity(this.affinity);
    while (running.get()) {
      cqService.wait_event(index);
    }
  }

  public void shutdown() {
    running.set(false); 
  }

  private int index;
  private CqService cqService;
  private long affinity;
  private final AtomicBoolean running = new AtomicBoolean(false);
}
