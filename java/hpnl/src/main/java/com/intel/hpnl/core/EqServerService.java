package com.intel.hpnl.core;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.Handler;

public class EqServerService extends EqService {
  private Handler connectedCallback;

  public EqServerService(int workerNum, int bufferNum, int bufferSize) {
    super(workerNum, bufferNum, bufferSize, true);
  }

  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    long ret = this.tryConnect(ip, port, cqIndex);
    this.connectedCallback = connectedCallback;
    return ret < 0L ? -1 : 0;
  }

  protected void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = (Connection)this.conMap.get(eq);
    if (this.connectedCallback != null) {
      this.connectedCallback.handle(connection, 0, 0);
    }

  }
}
