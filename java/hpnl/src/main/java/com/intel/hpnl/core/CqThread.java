package com.intel.hpnl.core;

import java.lang.Thread;
import java.util.concurrent.atomic.AtomicBoolean;

public class CqThread extends Thread {
  public CqThread(CqService cqService, int index) {
    this.cqService = cqService;
    this.index = index; 
    running.set(true);
  }

  public void run() {
    while (running.get()) {
      cqService.wait_cq_event(index);
    }
  }

  public void iterrupt() {
    running.set(false); 
  }

  private int index;
  private CqService cqService;
  private final AtomicBoolean running = new AtomicBoolean(false);
}
