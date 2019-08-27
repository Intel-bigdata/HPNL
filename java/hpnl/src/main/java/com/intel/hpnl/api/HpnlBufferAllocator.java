package com.intel.hpnl.api;

import io.netty.channel.AbstractChannel;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicInteger;

public class HpnlBufferAllocator {
    private final BufferCache<HpnlBuffer> CACHE_TINY;
    private final BufferCache<HpnlBuffer> CACHE_SMALL;
    private final BufferCache<HpnlBuffer> CACHE_MEDIUM;
    private final BufferCache<HpnlBuffer> CACHE_LARGE;

    private BufferCache.CacheHandler<HpnlBuffer> cacheHandler;

    private static final int MAX_SIZE = 10*1024*1024;

    public static final int BUFFER_TINY = 512;
    public static final int BUFFER_SMALL = 1024;
    public static final int BUFFER_MEDIUM = 4096;
    public static final int BUFFER_LARGE = 8192;

    private static final int MIN_DEFAULT_BUFFER_ID = -100000000;

    private static int currentBufferIdLimit = MIN_DEFAULT_BUFFER_ID;

    public static final int NUM_UNIT = 128;

    private static final BufferCache.CacheHandler<HpnlBuffer> DEFAULT_CACHE_HANDLER =
            new BufferCacheHandler(new AtomicInteger(-1), MIN_DEFAULT_BUFFER_ID);

    private static final HpnlBufferAllocator DEFAULT = new HpnlBufferAllocator(true, DEFAULT_CACHE_HANDLER);

    private HpnlBufferAllocator(boolean largePool, BufferCache.CacheHandler<HpnlBuffer> cacheHandler){
        this.cacheHandler = cacheHandler;
        CACHE_TINY = BufferCache.getInstance(cacheHandler, BUFFER_TINY,
                largePool?NUM_UNIT*8:NUM_UNIT*2, largePool?NUM_UNIT*8:NUM_UNIT*2);
        CACHE_SMALL = BufferCache.getInstance(cacheHandler, BUFFER_SMALL,
                largePool?NUM_UNIT*4:NUM_UNIT, largePool?NUM_UNIT*4:NUM_UNIT);
        CACHE_MEDIUM = BufferCache.getInstance(cacheHandler, BUFFER_MEDIUM,
                largePool?NUM_UNIT*2:NUM_UNIT, largePool?NUM_UNIT*2:NUM_UNIT);
        CACHE_LARGE = BufferCache.getInstance(cacheHandler, BUFFER_LARGE,
                largePool?NUM_UNIT*4:NUM_UNIT*2, largePool?NUM_UNIT*4:NUM_UNIT*2);
    }

    public static HpnlBuffer getBufferFromDefault(int size){
        return DEFAULT.getBuffer(size);
    }

    public static HpnlBuffer getBufferByIdFromDefault(int bufferId){
        return DEFAULT.getBufferById(bufferId);
    }

    public static HpnlBufferAllocator getInstance(){
        return getInstance(false);
    }

    public static HpnlBufferAllocator getInstance(boolean largePool){
        int start;
        synchronized (HpnlBufferAllocator.class) {
            start = currentBufferIdLimit;
            currentBufferIdLimit -= MIN_DEFAULT_BUFFER_ID;
            if(currentBufferIdLimit > 0){
                throw new IllegalStateException("reach buffer limit: "+currentBufferIdLimit);
            }
        }
        return new HpnlBufferAllocator(largePool,
                new BufferCacheHandler(new AtomicInteger(start - 1), start - MIN_DEFAULT_BUFFER_ID));
    }

    public HpnlBuffer getBuffer(int size){
        return getBuffer(this, size);
    }

    public HpnlBuffer getBufferById(int bufferId){
        if(bufferId > MIN_DEFAULT_BUFFER_ID){
            return getBufferById(DEFAULT, bufferId);
        }
        return getBufferById(this, bufferId);
    }

    /**
     * different sizes (512, 1024, 4096, 8192) in threadlocal
     * @return
     */
    private HpnlBuffer getBuffer(HpnlBufferAllocator allocator, int size){
        if(size <= BUFFER_TINY){
            HpnlBuffer buffer = allocator.CACHE_TINY.getInstance();
            buffer.clear();
            return buffer;
        }
        if(size <= BUFFER_SMALL){
            HpnlBuffer buffer = allocator.CACHE_SMALL.getInstance();
            buffer.clear();
            return buffer;
        }
        if(size <= BUFFER_MEDIUM){
            HpnlBuffer buffer = allocator.CACHE_MEDIUM.getInstance();
            buffer.clear();
            return buffer;
        }
        if(size <= BUFFER_LARGE){
            HpnlBuffer buffer = allocator.CACHE_LARGE.getInstance();
            buffer.clear();
            return buffer;
        }
        if(size > MAX_SIZE){
            throw new IllegalArgumentException("buffer size cannot exceed "+MAX_SIZE);
        }
        return DEFAULT_CACHE_HANDLER.newInstance(null, size);
    }

    private HpnlBuffer getBufferById(HpnlBufferAllocator allocator, int bufferId){
        HpnlBuffer buffer = allocator.CACHE_TINY.getBuffer(bufferId);
        if(buffer != null){
            return buffer;
        }
        buffer = allocator.CACHE_SMALL.getBuffer(bufferId);
        if(buffer != null){
            return buffer;
        }
        buffer = allocator.CACHE_MEDIUM.getBuffer(bufferId);
        if(buffer != null){
            return buffer;
        }
        buffer = allocator.CACHE_LARGE.getBuffer(bufferId);
        return buffer;
    }

    private static class BufferCacheHandler implements BufferCache.CacheHandler<HpnlBuffer>{
        private AtomicInteger idGen;
        private int idLimit;

        public BufferCacheHandler(AtomicInteger idGen, int idLimit){
            this.idGen = idGen;
            this.idLimit = idLimit;
        }
        @Override
        public HpnlBuffer newInstance(BufferCache<HpnlBuffer> cache, int size) {
            int id = 0;
            if(cache != null) {
                id = idGen.getAndDecrement();
                if (id < idLimit) {
                    throw new IllegalArgumentException("id should be less than " + idLimit);
                }
            }
            HpnlBuffer buffer = new HpnlGlobalBuffer(cache, id, size);
            return buffer;
        }
    }

    private static class HpnlGlobalBuffer extends AbstractHpnlBuffer{
        private BufferCache<HpnlBuffer> cache;
        private int size;

        public static final int METADATA_SIZE = 8 + BASE_METADATA_SIZE;

        public HpnlGlobalBuffer(BufferCache<HpnlBuffer> cache, int id, int size){
            super(id);
            this.cache = cache;
            this.size = size;
            byteBuffer = ByteBuffer.allocateDirect(size);
            if(cache != null){
                cache.putBuffer(id, this);
            }
        }

        @Override
        public ByteBuffer parse(int bufferSize) {
            this.byteBuffer.position(0);
            this.byteBuffer.limit(bufferSize);
            this.frameType = this.byteBuffer.get();
            peerConnectionId = this.byteBuffer.getLong();
            this.seq = this.byteBuffer.getLong();
            return this.byteBuffer.slice();
        }

        private void putMetadata(int srcSize, byte type, long seq) {
            this.byteBuffer.rewind();
            this.byteBuffer.limit(METADATA_SIZE + srcSize);
            this.byteBuffer.put(type);
            this.byteBuffer.putLong(this.connectionId);
            this.byteBuffer.putLong(seq);
        }

        @Override
        public void putData(ByteBuffer dataBuffer, byte frameType, long seqId) {
            this.putMetadata(dataBuffer.remaining(), frameType, seqId);
            this.byteBuffer.put(dataBuffer);
            this.byteBuffer.flip();
        }

        @Override
        public void insertMetadata(byte frameType, long seqId, int bufferLimit) {
            this.byteBuffer.position(0);
            this.byteBuffer.put(frameType);
            this.byteBuffer.position(byteBuffer.position()+8);
            this.byteBuffer.putLong(seqId);
            this.byteBuffer.limit(bufferLimit);
            this.byteBuffer.position(bufferLimit);
        }

        @Override
        public void setConnectionId(long connectionId) {
            this.byteBuffer.position(1);
            this.byteBuffer.putLong(connectionId);
            this.byteBuffer.position(0);
        }

        @Override
        public int getMetadataSize() {
            return METADATA_SIZE;
        }

        @Override
        public void clear() {
            super.clear();
            connectionId = -1;
            peerConnectionId = -1;
        }

        @Override
        public void release() {
            if(cache != null){
                cache.reclaim(this);
            }
        }

        @Override
        public BufferType getBufferType() {
            return BufferType.GLOBAL;
        }
    }
}
