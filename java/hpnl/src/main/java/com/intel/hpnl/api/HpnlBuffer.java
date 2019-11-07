package com.intel.hpnl.api;

import java.nio.ByteBuffer;

public interface HpnlBuffer {

  ByteBuffer parse(int bufferSize);

  byte getFrameType();

  ByteBuffer getRawBuffer();

  int remaining();

  int getBufferId();

  int capacity();

  int writtenDataSize();

  void putData(ByteBuffer dataBuffer, byte frameType);

  void insertMetadata(byte frameType);

  byte get();

  HpnlBuffer get(byte[] bytes);

  HpnlBuffer get(byte[] bytes, int offset, int length);

  int getInt();

  long getLong();

  void put(byte b);

  void put(ByteBuffer src);

  void put(ByteBuffer src, int length);

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

  int SEND_BUFFER_ID_START = 1;

  int RECV_BUFFER_ID_START = 1000000000;

  void received();

  long getMemoryAddress();

  enum BufferType{
    SEND, RECV, GLOBAL, RDMA
  }
}
