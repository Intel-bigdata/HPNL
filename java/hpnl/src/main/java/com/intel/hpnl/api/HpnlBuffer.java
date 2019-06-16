package com.intel.hpnl.api;

import java.nio.ByteBuffer;

public interface HpnlBuffer {
  ByteBuffer parse(int var1);

  byte getFrameType();

  ByteBuffer getRawBuffer();

  int remaining();

  int getBufferId();

  int capacity();

  long getSeq();

  void putData(ByteBuffer var1, byte var2, long var3);

  void insertMetadata(byte var1, long var2, int var4);

  int getMetadataSize();
}
