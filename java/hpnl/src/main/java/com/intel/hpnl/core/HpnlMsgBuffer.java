package com.intel.hpnl.core;

import com.intel.hpnl.api.AbstractHpnlBuffer;
import java.nio.ByteBuffer;

public class HpnlMsgBuffer extends AbstractHpnlBuffer {
  public static final int METADATA_SIZE = BASE_METADATA_SIZE;

  public HpnlMsgBuffer(int bufferId, ByteBuffer byteBuffer) {
    super(bufferId, byteBuffer);
  }

  private void putMetadata(int srcSize, byte type, long seq) {
    this.byteBuffer.rewind();
    this.byteBuffer.limit(9 + srcSize);
    this.byteBuffer.put(type);
    this.byteBuffer.putLong(seq);
  }

  public void putData(ByteBuffer src, byte type, long seq) {
    this.putMetadata(src.remaining(), type, seq);
    this.byteBuffer.put(src.slice());
    this.byteBuffer.flip();
  }

  public ByteBuffer parse(int blockBufferSize) {
    this.byteBuffer.position(0);
    this.byteBuffer.limit(blockBufferSize);
    this.frameType = this.byteBuffer.get();
    this.seq = this.byteBuffer.getLong();
    return this.byteBuffer.slice();
  }

  public void insertMetadata(byte frameType, long seqId, int limit) {
    this.byteBuffer.position(0);
    this.byteBuffer.put(frameType);
    this.byteBuffer.putLong(seqId);
    this.byteBuffer.position(limit);
  }

  public int getMetadataSize() {
    return METADATA_SIZE;
  }
}
