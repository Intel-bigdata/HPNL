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

  int sendBufferToAddress(HpnlBuffer buffer, int bufferSize, long peerAddress);

  int sendBufferToId(HpnlBuffer buffer, int bufferSize, long peerConnectId);

  int sendBuffer(HpnlBuffer buffer, int bufferSize) ;

  int sendConnectRequest(HpnlBuffer buffer, int bufferSize) ;

  int sendBufferToId(ByteBuffer buffer, int bufferSize, long peerConnectionId);

  int sendBuffer(ByteBuffer buffer, int bufferSize);

  String getSrcAddr();

  int getSrcPort();

  String getDestAddr();

  int getDestPort();

  ByteBuffer getLocalName();

  long resolvePeerName(ByteBuffer peerName);

  void putProviderAddress(long connectionId, long address);

  long getProviderAddress(long connectionId);

  void putPeerAddress(long connectId, Object[] address);

  Object[] getPeerAddress(long connectId);

  void shutdown();

  void adjustSendTarget(int sendCtxId);
}
