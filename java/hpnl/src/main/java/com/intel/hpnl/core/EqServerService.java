package com.intel.hpnl.core;

public class EqServerService extends EqService {

  private Handler connectedCallback;

  public EqServerService(int worker_num, int buffer_num) {
    super(worker_num, buffer_num, true);
  }

  @Override
  public void setConnectedCallback(Handler callback) {
    connectedCallback = callback;
  }

  @Override
  protected void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = conMap.get(eq);
    if(connectedCallback != null) {
      connectedCallback.handle(connection, 0, 0);
    }
  }
}
