package com.intel.hpnl.core;

import com.intel.hpnl.api.EventTask;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlService;
import com.intel.hpnl.api.ServiceException;
import com.intel.hpnl.api.HpnlService.EndpointType;
import java.util.List;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class MsgHpnlService implements HpnlService {
  private EqService eqService;
  private CqService cqService;
  private boolean server;
  private static final Logger logger = LoggerFactory.getLogger(MsgHpnlService.class);

  public MsgHpnlService(int numThreads, int numBuffers, int bufferSize, boolean server) {
    if (server) {
      this.eqService = (new EqServerService(numThreads, numBuffers, bufferSize)).init();
    } else {
      this.eqService = (new EqService(numThreads, numBuffers, bufferSize)).init();
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

  public int bind(String hostname, int port, int cqIndex, Handler connectedCallback) {
    return this.eqService.connect(hostname, String.valueOf(port), cqIndex, connectedCallback);
  }

  public int connect(String hostname, int port, int cqIndex, Handler connectedCallback) {
    return this.eqService.connect(hostname, String.valueOf(port), cqIndex, connectedCallback);
  }

  public boolean isServer() {
    return this.server;
  }

  public void stop() {
    this.cqService.stop();
    this.eqService.stop();
  }

  public EventTask getEqTask() {
    return this.eqService.getEventTask();
  }

  public List<EventTask> getCqTasks() {
    return this.cqService.getEventTasks();
  }

  public long getNewConnectionId() {
    return this.eqService.getNewConnectionId();
  }

  public void removeConnection(long connectionId, long connEq, boolean proactive) {
    this.eqService.removeConnection(connectionId, connEq, proactive);
  }

  public EndpointType getEndpointType() {
    return EndpointType.MSG;
  }
}
