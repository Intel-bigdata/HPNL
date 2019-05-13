package com.intel.hpnl.core;

public class EqServerService extends EqService {

  private Handler connectedCallback;

  public EqServerService(int worker_num, int buffer_num) {
    super(worker_num, buffer_num, true);
  }

  @Override
  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    long ret = setupConnection(ip, port, cqIndex, connectedCallback);
    this.connectedCallback = connectedCallback;
    return ret<0 ? -1:0;
  }

  @Override
  public int connect(String ip, String port, int cqIndex, long timeoutMill) {
    long ret = setupConnection(ip, port, cqIndex, null);
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
