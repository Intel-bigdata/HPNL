package com.intel.hpnl.api;

import java.nio.ByteBuffer;

public abstract class AbstractHpnlBuffer implements HpnlBuffer {
  private int bufferId;
  protected byte frameType;
  protected long seq;
  protected ByteBuffer byteBuffer;
  public static final int BASE_METADATA_SIZE = 9;

  protected AbstractHpnlBuffer(int bufferId, ByteBuffer byteBuffer) {
    this.bufferId = bufferId;
    this.byteBuffer = byteBuffer;
  }

  public int getBufferId() {
    return this.bufferId;
  }

  public byte getFrameType() {
    return this.frameType;
  }

  public long getSeq() {
    return this.seq;
  }

  public ByteBuffer getRawBuffer() {
    return this.byteBuffer;
  }

  public int getMetadataSize() {
    return 9;
  }

  public int remaining() {
    return this.byteBuffer.remaining();
  }

  public int capacity() {
    return this.byteBuffer.capacity();
  }
}
