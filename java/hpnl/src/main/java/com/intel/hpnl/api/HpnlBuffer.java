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

  void putData(ByteBuffer dataBuffer, byte frameType, long seqId);

  void insertMetadata(byte frameType, long seqId, int bufferLimit);

  int getMetadataSize();

  void clear();
}
