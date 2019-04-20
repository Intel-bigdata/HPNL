package com.intel.hpnl.service;

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.Handler;

public abstract class Service {
  public Service(int workNbr, int bufferNbr, boolean isServer) {
    this.eqService = new EqService(workNbr, bufferNbr, isServer).init();
    this.cqService = new CqService(this.eqService).init();
  }

  public void initBufferPool(int bufferNbr, int bufferSize, int nextBufferNbr) {
    this.eqService.initBufferPool(bufferNbr, bufferSize, nextBufferNbr);
  }

  public void setAffinities(int[] affinities) {
    this.cqService.setAffinities(affinities);
  }

  public void join() {
    this.cqService.join();
    this.eqService.shutdown();
    this.eqService.join();
  }

  public void setConnectedCallback(Handler callback) {
    this.eqService.setConnectedCallback(callback);
  }

  public void setRecvCallback(Handler callback) {
    this.eqService.setRecvCallback(callback);
  }

  public void setSendCallback(Handler callback) {
    this.eqService.setSendCallback(callback);
  }

  public void setReadCallback(Handler callback) {
    this.eqService.setReadCallback(callback);
  }

  public void setShutdownCallback(Handler callback) {
    this.eqService.setShutdownCallback(callback);
  }

  protected EqService eqService;
  protected CqService cqService;
}
