package com.intel.hpnl.service;

import com.intel.hpnl.core.Connection;

public class Client extends Service {
  public Client(int workNbr, int bufferNbr) {
    super(workNbr, bufferNbr, false);
  }

  public Connection connect(String ip, String port, int timeout) {
    this.cqService.start();
    return this.eqService.connect(ip, port, timeout);
  }
}
