package com.intel.hpnl.api;

import java.nio.ByteBuffer;

public abstract class AbstractHpnlBuffer implements HpnlBuffer {
  private int bufferId;
  protected byte frameType;
  protected long seq;
  protected long connectionId;
  protected long peerConnectionId;
  protected ByteBuffer byteBuffer;
  private BufferType bufferType;

  public static final int BASE_METADATA_SIZE = 9;

  protected AbstractHpnlBuffer(int bufferId, ByteBuffer byteBuffer, BufferType bufferType) {
    this.bufferId = bufferId;
    this.byteBuffer = byteBuffer;
    this.bufferType = bufferType;
  }

  protected AbstractHpnlBuffer(int bufferId){
    this.bufferId = bufferId;
  }

  @Override
  public int getBufferId() {
    return this.bufferId;
  }

  @Override
  public long getConnectionId() {
    return connectionId;
  }

  @Override
  public void setConnectionId(long connectionId) {
    this.connectionId = connectionId;
  }

  @Override
  public long getPeerConnectionId() {
    return peerConnectionId;
  }

  @Override
  public byte getFrameType() {
    return this.frameType;
  }

  @Override
  public long getSeq() {
    return this.seq;
  }

  @Override
  public ByteBuffer getRawBuffer() {
    return this.byteBuffer;
  }

  @Override
  public int getMetadataSize() {
    return BASE_METADATA_SIZE;
  }

  @Override
  public int remaining() {
    return this.byteBuffer.remaining();
  }

  @Override
  public int capacity() {
    return this.byteBuffer.capacity();
  }

  @Override
  public void clear(){
    this.byteBuffer.clear();
  }

  @Override
  public BufferType getBufferType() {
    return bufferType;
  }
}
