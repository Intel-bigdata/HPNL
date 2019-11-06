package com.intel.hpnl.api;

import java.nio.ByteBuffer;

/**
 * TODO: with unsafe
 */
public abstract class AbstractHpnlBuffer implements HpnlBuffer {
  private final int bufferId;
  protected byte frameType;
  protected ByteBuffer byteBuffer;
  private BufferType bufferType;

  private boolean parsed;

  public static final int BASE_METADATA_SIZE = HpnlBufferAllocator.BUFFER_METADATA_SIZE;

  protected AbstractHpnlBuffer(int bufferId, ByteBuffer byteBuffer, BufferType bufferType) {
    this.bufferId = bufferId;
    this.byteBuffer = byteBuffer;
    this.bufferType = bufferType;
  }

  protected AbstractHpnlBuffer(int bufferId){
    this.bufferId = bufferId;
  }

  private void putMetadata(int srcSize, byte type) {
    this.byteBuffer.rewind();
    this.byteBuffer.limit(BASE_METADATA_SIZE + srcSize);
    this.byteBuffer.put(type);
  }

  @Override
  public void putData(ByteBuffer dataBuffer, byte frameType) {
    this.putMetadata(dataBuffer.remaining(), frameType);
    this.byteBuffer.put(dataBuffer);
    this.byteBuffer.flip();
  }

  @Override
  public void insertMetadata(byte frameType) {
    int posBef = this.byteBuffer.position();
    this.byteBuffer.position(0);
    this.byteBuffer.put(frameType);
    this.byteBuffer.position(posBef);
  }

  @Override
  public int getBufferId() {
    return this.bufferId;
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
  public void put(ByteBuffer src, int length){
    int pos = src.position();
    for(int i=pos; i < pos + length; i++){
      byteBuffer.put(src.get());
    }
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
  public void received(){
    parsed = false;
  }

  @Override
  public ByteBuffer parse(int bufferSize) {
    if(parsed){
      return byteBuffer;
    }
    this.byteBuffer.position(0);
    this.byteBuffer.limit(bufferSize);
    this.frameType = this.byteBuffer.get();
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
  public int writtenDataSize(){
    return byteBuffer.position() - getMetadataSize();
  }

  @Override
  public int capacity() {
    return this.byteBuffer.capacity();
  }

  @Override
  public void clear(){
    this.byteBuffer.clear();
    parsed = false;
  }

  @Override
  public BufferType getBufferType() {
    return bufferType;
  }

  @Override
  public long getMemoryAddress(){
    return 0;
  }
}
