package com.intel.hpnl.api;

import java.nio.ByteBuffer;

public abstract class AbstractHpnlBuffer implements HpnlBuffer {
  private final int bufferId;
  protected byte frameType;
  protected long seq;
  protected long connectionId;
  protected long peerConnectionId;
  protected ByteBuffer byteBuffer;
  private BufferType bufferType;

  private boolean parsed;

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
  public byte get() {
    return byteBuffer.get();
  }

  @Override
  public HpnlBuffer get(byte[] bytes){
    byteBuffer.get(bytes);
    return this;
  }

  @Override
  public HpnlBuffer get(byte[] bytes, int offset, int length){
    byteBuffer.get(bytes, offset, length);
    return this;
  }

  @Override
  public int getInt() {
    return byteBuffer.getInt();
  }

  @Override
  public long getLong() {
    return byteBuffer.getLong();
  }

  @Override
  public void put(byte b) {
    byteBuffer.put(b);
  }

  @Override
  public void put(ByteBuffer src) {
    byteBuffer.put(src);
  }

  @Override
  public void put(byte[] src){
    byteBuffer.put(src);
  }

  @Override
  public void put(byte[] src, int offset, int length){
    byteBuffer.put(src, offset, length);
  }

  @Override
  public void putInt(int i) {
    byteBuffer.putInt(i);
  }

  @Override
  public void putLong(long l) {
    byteBuffer.putLong(l);
  }

  @Override
  public int position(){
    return byteBuffer.position();
  }

  @Override
  public void position(int pos){
    byteBuffer.position(pos);
  }

  @Override
  public void limit(int limit){
    byteBuffer.limit(limit);
  }

  @Override
  public int limit(){
    return byteBuffer.limit();
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
  public ByteBuffer parse(int bufferSize) {
    if(parsed){
      return byteBuffer;
    }
    this.byteBuffer.position(0);
    this.byteBuffer.limit(bufferSize);
    this.frameType = this.byteBuffer.get();
    peerConnectionId = this.byteBuffer.getLong();
    this.seq = this.byteBuffer.getLong();
    parsed = true;
    return this.byteBuffer;
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
    this.parsed = false;
    this.peerConnectionId = -1;
  }

  @Override
  public BufferType getBufferType() {
    return bufferType;
  }
}
