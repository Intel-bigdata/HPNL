package com.intel.hpnl.core;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.Handler;

public class RdmServerService extends RdmService {
  public RdmServerService(int workNum, int bufferNum, int bufferSize, int ioRatio) {
    super(workNum, bufferNum, bufferSize,  ioRatio, true);
  }

  @Override
  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    localIp = ip;
    localPort = Integer.valueOf(port);
    Connection connection = this.getConnection(this.listen(ip, port, this.getNativeHandle()));
    return connectedCallback.handle(connection, -1, -1);
  }
}
