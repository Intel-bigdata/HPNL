package com.intel.hpnl.api;

import java.nio.ByteBuffer;

public interface Connection {
  long getConnectionId();

  int getCqIndex();

  void setRecvCallback(Handler var1);

  void setReadCallback(Handler var1);

  void addShutdownCallback(Handler var1);

  HpnlBuffer getRecvBuffer(int var1);

  void releaseRecvBuffer(int var1);

  HpnlBuffer takeSendBuffer();

  HpnlBuffer getSendBuffer(int var1);

  void pushSendBuffer(HpnlBuffer var1);

  void pushRecvBuffer(HpnlBuffer var1);

  int send(int var1, int var2);

  int sendTo(int var1, int var2, ByteBuffer var3);

  String getSrcAddr();

  int getSrcPort();

  String getDestAddr();

  int getDestPort();

  ByteBuffer getLocalName();

  void shutdown();
}
