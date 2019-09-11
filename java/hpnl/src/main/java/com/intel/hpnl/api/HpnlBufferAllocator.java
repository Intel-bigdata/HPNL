package com.intel.hpnl.api;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicInteger;

public class HpnlBufferAllocator {
    private final BufferCache<HpnlBuffer> CACHE_TINY;
    private final BufferCache<HpnlBuffer> CACHE_SMALL;
    private final BufferCache<HpnlBuffer> CACHE_MEDIUM;
    private final BufferCache<HpnlBuffer> CACHE_LARGE;
    private BufferCache<HpnlBuffer> CACHE_MAXSIZE;

    private BufferCache.CacheHandler<HpnlBuffer> cacheHandler;
    private final boolean largePool;
    private int maxSize;

    public static final int BUFFER_TINY = 512;
    public static final int BUFFER_SMALL = 1024;
    public static final int BUFFER_MEDIUM = 4096;
    public static final int BUFFER_LARGE = 8192;

    private static final int BUFFER_ID_RANGES = 100000000;
    private static final int MIN_DEFAULT_BUFFER_ID = -BUFFER_ID_RANGES;

    private static int currentBufferIdLimit = MIN_DEFAULT_BUFFER_ID;

    public static final int NUM_UNIT = 128;

    public static final int BUFFER_METADATA_SIZE = 8 + AbstractHpnlBuffer.BASE_METADATA_SIZE;

    private static final BufferCache.CacheHandler<HpnlBuffer> DEFAULT_CACHE_HANDLER =
            new BufferCacheHandler(new AtomicInteger(-1), MIN_DEFAULT_BUFFER_ID);

    private static final HpnlBufferAllocator DEFAULT = new HpnlBufferAllocator(true, DEFAULT_CACHE_HANDLER);

    private HpnlBufferAllocator(boolean largePool, BufferCache.CacheHandler<HpnlBuffer> cacheHandler){
        this.largePool = largePool;
        this.cacheHandler = cacheHandler;
        HpnlConfig config = HpnlConfig.getInstance();
        int tinyNum = config.getBufferNumTiny();
        int smallNum = config.getBufferNumSmall();
        int mediumNum = config.getBufferNumMedium();
        int largeNum = config.getBufferNumLarge();

        CACHE_TINY = BufferCache.getInstance(cacheHandler, BUFFER_TINY,
                largePool?tinyNum*2:tinyNum, largePool?tinyNum*2:tinyNum);
        CACHE_SMALL = BufferCache.getInstance(cacheHandler, BUFFER_SMALL,
                largePool?smallNum:smallNum, largePool?smallNum:smallNum);
        CACHE_MEDIUM = BufferCache.getInstance(cacheHandler, BUFFER_MEDIUM,
                largePool?mediumNum:mediumNum, largePool?mediumNum:mediumNum);
        CACHE_LARGE = BufferCache.getInstance(cacheHandler, BUFFER_LARGE,
                largePool?largeNum*2:largeNum, largePool?largeNum*2:largeNum);
        this.maxSize = -1;
    }

    public static HpnlBuffer getBufferFromDefault(int size){
        return DEFAULT.getBuffer(size);
    }

//    public static HpnlBuffer getBufferByIdFromDefault(int bufferId){
//        return DEFAULT.getBufferById(bufferId);
//    }

    public static void setDefaultMaxBufferSize(int maxSize){
        setMaxBufferSize(DEFAULT, maxSize);
    }

    private static void setMaxBufferSize(HpnlBufferAllocator allocator, int maxSize){
        synchronized (allocator){
            if(allocator.CACHE_MAXSIZE == null){
                allocator.maxSize = maxSize;
                boolean largePool = allocator.largePool;
                allocator.CACHE_MAXSIZE = BufferCache.getInstance(allocator.cacheHandler, maxSize,
                        largePool?NUM_UNIT*2:NUM_UNIT, largePool?NUM_UNIT*2:NUM_UNIT);
            }
        }
    }

    public static HpnlBufferAllocator getInstance(){
        return getInstance(false);
    }

    public static HpnlBufferAllocator getInstance(boolean largePool){
        int start;
        synchronized (HpnlBufferAllocator.class) {
            start = currentBufferIdLimit;
            currentBufferIdLimit -= BUFFER_ID_RANGES;
            if(currentBufferIdLimit > 0){
                throw new IllegalStateException("reach buffer limit: "+currentBufferIdLimit);
            }
        }
        return new HpnlBufferAllocator(largePool,
                new BufferCacheHandler(new AtomicInteger(start - 1), start - BUFFER_ID_RANGES));
    }

    public void setMaxBufferSize(int maxSize){
        setMaxBufferSize(this, maxSize);
    }

    public HpnlBuffer getBuffer(int size){
        return getBuffer(this, size);
    }

//    public HpnlBuffer getBufferById(int bufferId){
//        if(bufferId >= MIN_DEFAULT_BUFFER_ID){
//            return getBufferById(DEFAULT, bufferId);
//        }
//        return getBufferById(this, bufferId);
//    }

    /**
     * different sizes (512, 1024, 4096, 8192, maxSize) in threadlocal
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
        if(maxSize == -1){
            throw new IllegalStateException("please set max buffer size first");
        }
        if(size > maxSize){
            throw new IllegalArgumentException("buffer size cannot exceed "+maxSize);
        }

        HpnlBuffer buffer = allocator.CACHE_MAXSIZE.getInstance();
        buffer.clear();
        return buffer;
    }

//    private HpnlBuffer getBufferById(HpnlBufferAllocator allocator, int bufferId){
//        HpnlBuffer buffer = allocator.CACHE_TINY.getBuffer(bufferId);
//        if(buffer != null){
//            return buffer;
//        }
//        buffer = allocator.CACHE_SMALL.getBuffer(bufferId);
//        if(buffer != null){
//            return buffer;
//        }
//        buffer = allocator.CACHE_MEDIUM.getBuffer(bufferId);
//        if(buffer != null){
//            return buffer;
//        }
//        buffer = allocator.CACHE_LARGE.getBuffer(bufferId);
//        if(buffer != null) {
//            return buffer;
//        }
//        if(maxSize == -1){
//            throw new IllegalStateException("please set max buffer size first");
//        }
//        return allocator.CACHE_MAXSIZE.getBuffer(bufferId);
//    }

    private static class BufferCacheHandler implements BufferCache.CacheHandler<HpnlBuffer>{
        private AtomicInteger idGen;
        private int idLimit;

        public BufferCacheHandler(AtomicInteger idGen, int idLimit){
            this.idGen = idGen;
            this.idLimit = idLimit;
        }
        @Override
        public HpnlBuffer newInstance(BufferCache<HpnlBuffer> cache, int size) {
            int id = idGen.getAndDecrement();
            if (id < idLimit) {
                throw new IllegalArgumentException("id should not be less than " + idLimit);
            }
            HpnlBuffer buffer = new HpnlGlobalBuffer(cache, id, size);
            return buffer;
        }
    }

    public static class HpnlGlobalBuffer extends AbstractHpnlBuffer{
        private BufferCache<HpnlBuffer> cache;
        private volatile boolean released;

        public static final int METADATA_SIZE = BUFFER_METADATA_SIZE;

        public HpnlGlobalBuffer(BufferCache<HpnlBuffer> cache, int id, int size){
            super(id);
            this.cache = cache;
            byteBuffer = ByteBuffer.allocateDirect(size);
            if(cache != null){
                cache.putBuffer(id, this);
            }
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
            this.byteBuffer.position(bufferLimit);
        }

        @Override
        public void setConnectionId(long connectionId) {
            int pos = byteBuffer.position();
            this.byteBuffer.position(1);
            this.byteBuffer.putLong(connectionId);
            this.byteBuffer.position(pos);
        }

        @Override
        public int getMetadataSize() {
            return METADATA_SIZE;
        }

        @Override
        public void clear() {
            super.clear();
            connectionId = -1;
            released = false;
        }

        @Override
        public void release() {
            if(cache != null && !released){
                cache.reclaim(this);
            }
            released = true;
        }

        @Override
        public BufferType getBufferType() {
            return BufferType.GLOBAL;
        }
    }
}
