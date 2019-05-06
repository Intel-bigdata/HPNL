package com.intel.hpnl.service;

import java.nio.ByteBuffer;
import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.HpnlBuffer;
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

  public void start() {
    this.cqService.start();
  }

  public void shutdown() {
    this.cqService.shutdown();
    this.cqService.join();
    this.eqService.shutdown();
    this.eqService.join();
  }

  public void join() {
    this.cqService.join();
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

  public HpnlBuffer regRmaBuffer(ByteBuffer byteBuffer, int bufferSize) {
    return this.eqService.regRmaBuffer(byteBuffer, bufferSize);
  }

  public HpnlBuffer regRmaBufferByAddress(ByteBuffer byteBuffer, long address, long bufferSize) {
    return this.eqService.regRmaBufferByAddress(byteBuffer, address, bufferSize);
  }

  public void unregRmaBuffer(int bufferId) {
    this.eqService.unregRmaBuffer(bufferId);
  }

  public HpnlBuffer getRmaBuffer(int bufferSize) {
    return this.eqService.getRmaBuffer(bufferSize);
  }

  public ByteBuffer getRmaBufferByBufferId(int rmaBufferId) {
    return this.eqService.getRmaBufferByBufferId(rmaBufferId);
  }

  protected EqService eqService;
  protected CqService cqService;
}
