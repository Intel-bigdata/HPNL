package com.intel.hpnl.core;

import com.intel.hpnl.api.EventTask;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlService;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicLong;

public class RdmHpnlService implements HpnlService {
  private RdmService service;
  private boolean server;
  private AtomicLong nextConnectionId = new AtomicLong(0L);

  public RdmHpnlService(int numThreads, int numBuffers, int bufferSize, int ioRatio, boolean server) {
    if (server) {
      this.service = (new RdmServerService(numThreads, numBuffers, bufferSize, ioRatio)).init();
    } else {
      this.service = (new RdmService(numThreads, numBuffers, bufferSize, ioRatio)).init();
    }
    this.service.setHpnlService(this);
    this.service.start();
    this.server = server;
  }

  @Override
  public int bind(String hostname, int port, int cqIndex, Handler connectedCallback) {
    return this.service.connect(hostname, String.valueOf(port), cqIndex, connectedCallback);
  }

  @Override
  public int connect(String hostname, int port, int cqIndex, Handler connectedCallback) {
    return this.service.connect(hostname, String.valueOf(port), cqIndex, connectedCallback);
  }

  @Override
  public void stop() {
    this.service.stop();
  }

  @Override
  public EventTask getEqTask() {
    return null;
  }

  public RdmService getRdmService(){
    return service;
  }

  @Override
  public List<EventTask> getCqTasks() {
    List<EventTask> tasks = new ArrayList();
    tasks.add(this.service.getEventTask());
    return tasks;
  }

  @Override
  public long getNewConnectionId() {
    return this.nextConnectionId.getAndIncrement();
  }

  @Override
  public void removeNativeConnection(long nativeConnectionId, long connEq, boolean proactive) {
    this.service.removeConnection(nativeConnectionId, connEq, proactive);
  }

  @Override
  public boolean isServer() {
    return this.server;
  }

  @Override
  public int getFreePort() {
    return service.getFreePort();
  }

  @Override
  public void reclaimPort(int port) {
    service.reclaimPort(port);
  }

  @Override
  public EndpointType getEndpointType() {
    return EndpointType.RDM;
  }
}
