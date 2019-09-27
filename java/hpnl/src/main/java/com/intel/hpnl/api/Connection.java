package com.intel.hpnl.api;

import java.nio.ByteBuffer;

public interface Connection {
  long getConnectionId();

  int getCqIndex();

  void setRecvCallback(Handler callback);

  void setReadCallback(Handler callback);

  void setConnectedCallback(Handler callback);

  void addShutdownCallback(Handler callback);

  void setHpnlBufferAllocator(HpnlBufferAllocator allocator);

  HpnlBuffer getRecvBuffer(int bufferId);

  void reclaimRecvBuffer(int bufferId);

  HpnlBuffer takeSendBuffer();

  HpnlBuffer getSendBuffer(int bufferId);

  void pushSendBuffer(HpnlBuffer buffer);

  void pushRecvBuffer(HpnlBuffer buffer);

  int sendBuffer(HpnlBuffer buffer, int bufferSize) ;

  int sendConnectRequest(HpnlBuffer buffer, int bufferSize) ;

  int sendBuffer(ByteBuffer buffer, int bufferSize);

  String getSrcAddr();

  int getSrcPort();

  String getDestAddr();

  int getDestPort();

  ByteBuffer getLocalName();

  long resolvePeerName(ByteBuffer peerName);

  void shutdown();

  void adjustSendTarget(int sendCtxId);
}
