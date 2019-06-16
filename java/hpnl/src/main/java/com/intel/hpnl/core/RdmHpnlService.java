package com.intel.hpnl.core;

import com.intel.hpnl.api.EventTask;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlService;
import com.intel.hpnl.api.HpnlService.EndpointType;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicLong;

public class RdmHpnlService implements HpnlService {
  private RdmService service;
  private boolean server;
  private AtomicLong nextConnectionId = new AtomicLong(0L);

  public RdmHpnlService(int numThreads, int numBuffers, int bufferSize, boolean server) {
    if (server) {
      this.service = (new RdmServerService(numThreads, numBuffers, bufferSize)).init();
    } else {
      this.service = (new RdmService(numThreads, numBuffers, bufferSize)).init();
    }

    this.service.setHpnlService(this);
    this.server = server;
  }

  public int bind(String hostname, int port, int cqIndex, Handler connectedCallback) {
    return this.service.connect(hostname, String.valueOf(port), cqIndex, connectedCallback);
  }

  public int connect(String hostname, int port, int cqIndex, Handler connectedCallback) {
    return this.service.connect(hostname, String.valueOf(port), cqIndex, connectedCallback);
  }

  public void stop() {
    this.service.stop();
  }

  public EventTask getEqTask() {
    return null;
  }

  public List<EventTask> getCqTasks() {
    List<EventTask> tasks = new ArrayList();
    tasks.add(this.service.getEventTask());
    return tasks;
  }

  public long getNewConnectionId() {
    return this.nextConnectionId.getAndIncrement();
  }

  public void removeConnection(long connEq, boolean proactive) {
    this.service.removeConnection(connEq, proactive);
  }

  public boolean isServer() {
    return this.server;
  }

  public EndpointType getEndpointType() {
    return EndpointType.RDM;
  }
}
