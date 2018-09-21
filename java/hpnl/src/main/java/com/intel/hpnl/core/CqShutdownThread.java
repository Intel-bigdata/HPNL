package com.intel.hpnl.core;

public class CqShutdownThread extends Thread {

  public CqShutdownThread(EqService eqService, CqService cqService) {
    this.eqService = eqService;
    this.cqService = cqService;
  }

  public void run() {
    this.cqService.shutdown();
    this.cqService.join();
    this.eqService.shutdown();
    this.eqService.join();
  } 

  private EqService eqService;
  private CqService cqService;
}
