package com.intel.hpnl.core;

import java.nio.ByteBuffer;

public class HpnlBuffer {
  public HpnlBuffer(int bufferId, ByteBuffer byteBuffer) {
    this.bufferId = bufferId;
    this.byteBuffer = byteBuffer;
  }

  public HpnlBuffer(int bufferId, ByteBuffer byteBuffer, long rkey) {
    this.bufferId = bufferId;
    this.byteBuffer = byteBuffer;
    this.rkey = rkey;
  }

  public HpnlBuffer(int bufferId, ByteBuffer byteBuffer, long rkey, long address) {
    this.bufferId = bufferId;
    this.byteBuffer = byteBuffer;
    this.rkey = rkey;
    this.address = address;
  }

  public int getBufferId() {
    return this.bufferId;
  }

  public byte getType() {
    return this.type;
  }

  public long getSeq() {
    return this.seq; 
  }

  public ByteBuffer getRawBuffer() {
    return this.byteBuffer;
  }

  public long getRKey() {
    return this.rkey;
  }

  public long getAddress() {
    return address;
  }

  public int size() {
    return this.byteBuffer.remaining();
  }

  private void putMetadata(int srcSize, byte type, long seq) {
    byteBuffer.rewind();
    byteBuffer.limit(getMetadataSize()+srcSize);
    byteBuffer.put(type);
    byteBuffer.putLong(seq);
  }

  public void put(ByteBuffer src, byte type, long seq) {
    putMetadata(src.remaining(), type, seq);
    byteBuffer.put(src.slice());
    byteBuffer.flip();
  }

  public ByteBuffer get(int blockBufferSize) {
    byteBuffer.position(0); 
    byteBuffer.limit(blockBufferSize);
    this.type = byteBuffer.get();
    this.seq = byteBuffer.getLong();
    return byteBuffer.slice();
  }

  public int getWritableBytes(){
    return this.byteBuffer.capacity() - getMetadataSize();
  }

  public static int getMetadataSize(){
    return 9;
  }

  private int bufferId;
  private byte type;
  private long seq;
  private ByteBuffer byteBuffer;
  private long rkey;
  private long address;
}
