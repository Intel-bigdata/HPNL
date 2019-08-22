package com.intel.hpnl.api;

import java.nio.ByteBuffer;
import java.util.concurrent.BlockingQueue;

public interface Connection {
  long getConnectionId();

  int getCqIndex();

  void setRecvCallback(Handler callback);

  void setReadCallback(Handler callback);

  void setConnectedCallback(Handler callback);

  void addShutdownCallback(Handler callback);

  HpnlBuffer getRecvBuffer(int bufferId);

  void reclaimRecvBuffer(int bufferId);

  HpnlBuffer takeSendBuffer();

  HpnlBuffer getSendBuffer(int bufferId);

  void pushSendBuffer(HpnlBuffer buffer);

  void pushRecvBuffer(HpnlBuffer buffer);

  void setEventQueue(BlockingQueue<Runnable> eventQueue);

  int send(int bufferSize, int bufferId);

  int sendTo(int bufferSize, int bufferId, ByteBuffer peerName);

  int sendTo(int bufferSize, int bufferId, long connectionId);

  int sendBufferTo(ByteBuffer buffer, int bufferSize, long connectionId);

  int sendBuffer(ByteBuffer buffer, int bufferSize) ;

  String getSrcAddr();

  int getSrcPort();

  String getDestAddr();

  int getDestPort();

  ByteBuffer getLocalName();

  void putPeerName(long connectionId, ByteBuffer peer);

  ByteBuffer getPeerName(long connectionId);

  void putPeerAddress(long connectId, Object[] address);

  Object[] getPeerAddress(long connectId);

  void shutdown();
}
