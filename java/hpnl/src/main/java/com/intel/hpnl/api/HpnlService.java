package com.intel.hpnl.api;

import java.util.List;

public interface HpnlService {

  int bind(String hostname, int port, int cqIndex, Handler connectedCallback, Handler recvCallback);

  int connect(String hostname, int port, int cqIndex, Handler connectedCallback, Handler recvCallback);

  void stop();

  EventTask getCqTask(int cqIndex);

  List<EventTask> getCqTasks();

  long getNewConnectionId();

  void removeNativeConnection(long nativeConnectionId, long connHandle, boolean proactive);

  boolean isServer();

  int getFreePort();

  void reclaimPort(int port);

  Connection getConnection();

  HpnlService.EndpointType getEndpointType();

  HpnlBuffer getRecvBuffer(int bufferId);

  enum EndpointType {
    MSG,
    RDM
  }
}
