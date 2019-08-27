package com.intel.hpnl.api;

import java.nio.ByteBuffer;

public interface HpnlBuffer {

  ByteBuffer parse(int bufferSize);

  byte getFrameType();

  ByteBuffer getRawBuffer();

  int remaining();

  int getBufferId();

  int capacity();

  long getSeq();

  long getConnectionId();

  void setConnectionId(long connectionId);

  long getPeerConnectionId();

  void putData(ByteBuffer dataBuffer, byte frameType, long seqId);

  void insertMetadata(byte frameType, long seqId, int bufferLimit);

  int getMetadataSize();

  void clear();

  void release();

  BufferType getBufferType();

  enum BufferType{
    SEND, RECV, GLOBAL
  }
}
