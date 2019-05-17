package com.intel.hpnl.core;

/**
 * A server service for libfabric's event queue
 */
public class EqServerService extends EqService {

  private Handler connectedCallback;

  public EqServerService(int workerNum, int bufferNum, int bufferSize) {
    super(workerNum, bufferNum, bufferSize,true);
  }

  @Override
  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    long ret = tryConnect(ip, port, cqIndex);
    this.connectedCallback = connectedCallback;
    return ret<0 ? -1:0;
  }

  @Override
  protected void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = conMap.get(eq);
    if(connectedCallback != null) {
      connectedCallback.handle(connection, 0, 0);
    }
  }
}
