package com.intel.hpnl.api;

import java.util.List;

public interface HpnlService {

  int bind(String hostname, int port, int cqIndex, Handler connectedCallback);

  int connect(String hostname, int port, int cqIndex, Handler connectedCallback);

  void stop();

  EventTask getEqTask();

  List<EventTask> getCqTasks();

  long getNewConnectionId();

  void removeNativeConnection(long nativeConnectionId, long connHandle, boolean proactive);

  boolean isServer();

  int getFreePort();

  void reclaimPort(int port);

  HpnlService.EndpointType getEndpointType();

  enum EndpointType {
    MSG,
    RDM
  }
}
