package com.intel.hpnl.core;

import com.intel.hpnl.api.EventTask;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlService;
import com.intel.hpnl.api.ServiceException;
import java.util.List;
import java.util.concurrent.BlockingQueue;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class MsgHpnlService implements HpnlService {
  private EqService eqService;
  private CqService cqService;
  private boolean server;
  private static final Logger logger = LoggerFactory.getLogger(MsgHpnlService.class);

  public MsgHpnlService(int numThreads, int numBuffers, int bufferSize, int ioRatio, boolean server) {
    if (server) {
      this.eqService = (new EqServerService(numThreads, numBuffers, bufferSize, ioRatio)).init();
    } else {
      this.eqService = (new EqService(numThreads, numBuffers, bufferSize, ioRatio)).init();
    }

    this.checkInit(this.eqService, "failed to initialize EQ service");
    this.eqService.setHpnlService(this);
    this.cqService = (new CqService(this.eqService, this.eqService.getNativeHandle())).init();
    this.checkInit(this.cqService, "failed to initialize CQ service");
    this.server = server;
    logger.info("buffer count: {}, number of worker: {}", numBuffers, numThreads);
  }

  private void checkInit(Object object, String msg) {
    if (object == null) {
      throw new ServiceException(msg);
    }
  }

  @Override
  public int bind(String hostname, int port, int cqIndex, Handler connectedCallback, Handler recvCallback) {
    return this.eqService.connect(hostname, String.valueOf(port), cqIndex, connectedCallback, recvCallback);
  }

  @Override
  public int connect(String hostname, int port, int cqIndex, Handler connectedCallback, Handler recvCallback) {
    return this.eqService.connect(hostname, String.valueOf(port), cqIndex, connectedCallback, recvCallback);
  }

  @Override
  public void startCq(int cqIndex, BlockingQueue<Runnable> queue) {
    
  }

  @Override
  public boolean isServer() {
    return this.server;
  }

  @Override
  public int getFreePort() {
    //TODO:
    return 0;
  }

  @Override
  public void reclaimPort(int port) {
    //TODO:
  }

  @Override
  public void stop() {
    this.cqService.stop();
    this.eqService.stop();
  }

  @Override
  public EventTask getEqTask() {
    return this.eqService.getEventTask();
  }

  @Override
  public List<EventTask> getCqTasks() {
    return this.cqService.getEventTasks();
  }

  @Override
  public long getNewConnectionId() {
    return this.eqService.getNewConnectionId();
  }

  @Override
  public void removeNativeConnection(long nativeConnectionId, long connEq, boolean proactive) {
    this.eqService.removeConnection(nativeConnectionId, connEq, proactive);
  }

  @Override
  public EndpointType getEndpointType() {
    return EndpointType.MSG;
  }
}
