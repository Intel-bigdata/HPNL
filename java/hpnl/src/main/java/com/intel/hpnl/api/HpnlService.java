package com.intel.hpnl.api;

import java.util.List;

public interface HpnlService {
  int bind(String var1, int var2, int var3, Handler var4);

  int connect(String var1, int var2, int var3, Handler var4);

  void stop();

  EventTask getEqTask();

  List<EventTask> getCqTasks();

  long getNewConnectionId();

  void removeConnection(long var1, boolean var3);

  boolean isServer();

  HpnlService.EndpointType getEndpointType();

  enum EndpointType {
    MSG,
    RDM
  }
}
