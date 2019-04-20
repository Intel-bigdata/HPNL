package com.intel.hpnl.service;

public class Server extends Service {
  public Server(int workNbr, int bufferNbr) {
    super(workNbr, bufferNbr, true);
  }

  public void listen(String ip, String port) {
    this.cqService.start();
    this.eqService.listen(ip, port);
  }
}
