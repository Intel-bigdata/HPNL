package com.intel.hpnl.core;

import com.intel.hpnl.api.AbstractHpnlBuffer;
import java.nio.ByteBuffer;

public class HpnlMsgBuffer extends AbstractHpnlBuffer {
  public static final int METADATA_SIZE = BASE_METADATA_SIZE;

  public HpnlMsgBuffer(int bufferId, ByteBuffer byteBuffer, BufferType type) {
    super(bufferId, byteBuffer, type);
  }

  private void putMetadata(int srcSize, byte type, long seq) {
    this.byteBuffer.rewind();
    this.byteBuffer.limit(9 + srcSize);
    this.byteBuffer.put(type);
    this.byteBuffer.putLong(seq);
  }

  @Override
  public void putData(ByteBuffer src, byte type, long seq) {
    this.putMetadata(src.remaining(), type, seq);
    this.byteBuffer.put(src.slice());
    this.byteBuffer.flip();
  }

  @Override
  public ByteBuffer parse(int blockBufferSize) {
    this.byteBuffer.position(0);
    this.byteBuffer.limit(blockBufferSize);
    this.frameType = this.byteBuffer.get();
    this.seq = this.byteBuffer.getLong();
    return this.byteBuffer.slice();
  }

  @Override
  public void insertMetadata(byte frameType, long seqId, int limit) {
    this.byteBuffer.position(0);
    this.byteBuffer.put(frameType);
    this.byteBuffer.putLong(seqId);
    this.byteBuffer.limit(limit);
    this.byteBuffer.position(limit);
  }

  @Override
  public int getMetadataSize() {
    return METADATA_SIZE;
  }

  @Override
  public long getConnectionId(){
    return -1L;
  }

  @Override
  public long getPeerConnectionId(){
    return -1L;
  }

  @Override
  public void release() {}
}
