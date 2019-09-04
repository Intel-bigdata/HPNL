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

  byte get();

  HpnlBuffer get(byte[] bytes);

  HpnlBuffer get(byte[] bytes, int offset, int length);

  int getInt();

  long getLong();

  void put(byte b);

  void put(ByteBuffer src);

  void put(byte[] src);

  void put(byte[] src, int offset, int length);

  void putInt(int i);

  void putLong(long l);

  int position();

  void position(int pos);

  void limit(int limit);

  int limit();

  int getMetadataSize();

  void clear();

  void release();

  BufferType getBufferType();

  enum BufferType{
    SEND, RECV, GLOBAL
  }
}
