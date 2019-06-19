package com.intel.hpnl.api;

import java.nio.ByteBuffer;

public interface Connection {
  long getConnectionId();

  int getCqIndex();

  void setRecvCallback(Handler callback);

  void setReadCallback(Handler callback);

  void addShutdownCallback(Handler callback);

  HpnlBuffer getRecvBuffer(int bufferId);

  void releaseRecvBuffer(int bufferId);

  HpnlBuffer takeSendBuffer();

  HpnlBuffer getSendBuffer(int bufferId);

  void pushSendBuffer(HpnlBuffer buffer);

  void pushRecvBuffer(HpnlBuffer buffer);

  int send(int bufferSize, int bufferId);

  int sendTo(int bufferSize, int bufferId, ByteBuffer peerName);

  String getSrcAddr();

  int getSrcPort();

  String getDestAddr();

  int getDestPort();

  ByteBuffer getLocalName();

  void shutdown();
}
