package com.intel.hpnl.core;

import java.util.concurrent.atomic.AtomicBoolean;

public class EqThread extends Thread {
  public EqThread(EqService service_) {
    this.service = service_;
    running.set(true);
  }

  public void run() {
    while (running.get() || service.needReap()) {
      this.service.wait_eq_event();
      this.service.externalEvent();
    }
  }

  
  public void shutdown() {
    running.set(false); 
  }

  private EqService service;
  private final AtomicBoolean running = new AtomicBoolean(false);
}
