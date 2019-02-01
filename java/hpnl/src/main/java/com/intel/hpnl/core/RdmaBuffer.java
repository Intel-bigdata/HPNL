package com.intel.hpnl.core;

import java.nio.ByteBuffer;

public class RdmaBuffer {
  public RdmaBuffer(int rdmaBufferId, ByteBuffer byteBuffer) {
    this.rdmaBufferId = rdmaBufferId;
    this.byteBuffer = byteBuffer;
  }

  public RdmaBuffer(int rdmaBufferId, ByteBuffer byteBuffer, long rkey) {
    this.rdmaBufferId = rdmaBufferId;
    this.byteBuffer = byteBuffer;
    this.rkey = rkey;
  }

  public RdmaBuffer(int rdmaBufferId, ByteBuffer byteBuffer, long rkey, long address) {
    this.rdmaBufferId = rdmaBufferId;
    this.byteBuffer = byteBuffer;
    this.rkey = rkey;
    this.address = address;
  }

  public int getRdmaBufferId() {
    return this.rdmaBufferId;
  }

  public byte getType() {
    return this.type;
  }

  public int getBlockBufferId() {
    return this.blockBufferId; 
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

  public int remaining() {
    return this.byteBuffer.remaining();
  }

  public void putMetadata(int srcSize, byte type, int blockBufferId, long seq) {
    byteBuffer.rewind();
    byteBuffer.limit(13+srcSize);
    byteBuffer.put(type);
    byteBuffer.putInt(blockBufferId);
    byteBuffer.putLong(seq);
  }

  public void put(ByteBuffer src, byte type, int blockBufferId, long seq) {
    putMetadata(src.remaining(), type, blockBufferId, seq);
    byteBuffer.put(src.slice());
    byteBuffer.flip();
  }

  public ByteBuffer get(int blockBufferSize) {
    byteBuffer.position(0); 
    byteBuffer.limit(blockBufferSize);
    this.type = byteBuffer.get();
    this.blockBufferId = byteBuffer.getInt();
    this.seq = byteBuffer.getLong();
    return byteBuffer.slice();
  }

  public int getWritableBytes(){
    return this.byteBuffer.capacity() - getMetadataSize();
  }

  public static int getMetadataSize(){
    return 13;
  }

  public void release() {

  }

  private int rdmaBufferId;
  private byte type;
  private int blockBufferId;
  private long seq;
  private ByteBuffer byteBuffer;
  private long rkey;
  private long address;
}
