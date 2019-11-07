package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlRdmaBuffer;
import io.netty.buffer.ByteBuf;
import sun.misc.Unsafe;
import sun.nio.ch.DirectBuffer;

import java.lang.reflect.Field;
import java.nio.ByteBuffer;

public class HpnlNettyBuffer implements HpnlRdmaBuffer {
    private int bufferId;
    private ByteBuf byteBuf;
    private boolean parsed;

    private static Unsafe unsafe;

    static{
        try {
            Field unsafeField = Unsafe.class.getDeclaredField("theUnsafe");
            unsafeField.setAccessible(true);
            unsafe = (Unsafe) unsafeField.get(null);
        }catch(Exception e){
            unsafe = null;
        }
    }

    public HpnlNettyBuffer(int bufferId, ByteBuf byteBuf) {
        if(!byteBuf.isDirect()){
            throw new IllegalArgumentException("need to be direct buffer");
        }
        this.bufferId = bufferId;
        this.byteBuf = byteBuf;
    }

    @Override
    public ByteBuf getRawRdmaBuffer() {
        return byteBuf;
    }

    @Override
    public ByteBuffer parse(int bufferSize) {
        if(parsed){
            return null;
        }
        //TODO: parse
        parsed = true;
        return null;
    }

    @Override
    public byte getFrameType() {
        throw new UnsupportedOperationException();
    }

    @Override
    public ByteBuffer getRawBuffer() {
        throw new UnsupportedOperationException();
    }

    @Override
    public int remaining() {
        return byteBuf.writableBytes();
    }

    @Override
    public int getBufferId() {
        return bufferId;
    }

    @Override
    public int capacity() {
        return byteBuf.capacity();
    }

    @Override
    public int writtenDataSize() {
        return byteBuf.writerIndex();
    }

    @Override
    public void putData(ByteBuffer dataBuffer, byte frameType) {
        byteBuf.writeBytes(dataBuffer);
    }

    @Override
    public void insertMetadata(byte frameType) {
        throw new UnsupportedOperationException();
    }

    @Override
    public byte get() {
        return byteBuf.readByte();
    }

    @Override
    public HpnlRdmaBuffer get(byte[] bytes) {
        byteBuf.getBytes(byteBuf.readerIndex(), bytes);
        byteBuf.readerIndex(Math.min(byteBuf.writerIndex(), byteBuf.readerIndex()+bytes.length));
        return this;
    }

    @Override
    public HpnlRdmaBuffer get(byte[] bytes, int offset, int length) {
        byteBuf.getBytes(byteBuf.readerIndex(), bytes, offset, length);
        byteBuf.readerIndex(Math.min(byteBuf.writerIndex(), byteBuf.readerIndex()+length));
        return this;
    }

    @Override
    public int getInt() {
        return byteBuf.readInt();
    }

    @Override
    public long getLong() {
        return byteBuf.readLong();
    }

    @Override
    public void put(byte b) {
        byteBuf.writeByte(b);
    }

    @Override
    public void put(ByteBuffer src) {
        byteBuf.writeBytes(src);
    }

    @Override
    public void put(ByteBuffer src, int length) {
        if(unsafe != null && src.isDirect()) {
            if(length < 0){
                throw new IllegalArgumentException("length should be no less than 0, "+length);
            }
            if(src.remaining() < length || remaining() < length){
                throw new IllegalArgumentException("both source buffer and dest buffer's remaining should no less than length, "+length);
            }
            unsafe.copyMemory(((DirectBuffer)src).address()+src.position(),
                    getMemoryAddress()+byteBuf.writerIndex(), length);
            src.position(src.position()+length);
            byteBuf.writerIndex(byteBuf.writerIndex()+length);
            return;
        }
        int pos = src.position();
        for (int i = pos; i < pos + length; i++) {
            byteBuf.writeByte(src.get());
        }
    }

    @Override
    public void put(byte[] src) {
        byteBuf.writeBytes(src);
    }

    @Override
    public void put(byte[] src, int offset, int length) {
        byteBuf.writeBytes(src, offset, length);
    }

    @Override
    public void putInt(int i) {
        byteBuf.writeInt(i);
    }

    @Override
    public void putLong(long l) {
        byteBuf.writeLong(l);
    }

    @Override
    public int position() {
        return byteBuf.readerIndex();
    }

    @Override
    public void position(int pos) {
        byteBuf.readerIndex(pos);
    }

    @Override
    public void limit(int limit) {
        byteBuf.writerIndex(limit);
    }

    @Override
    public int limit() {
        return byteBuf.writerIndex();
    }

    @Override
    public int getMetadataSize() {
        return 0;
    }

    @Override
    public void clear() {
        byteBuf.clear();
    }

    @Override
    public void release() {
        byteBuf.release();
    }

    @Override
    public void received() {
        parsed = false;
    }
}
