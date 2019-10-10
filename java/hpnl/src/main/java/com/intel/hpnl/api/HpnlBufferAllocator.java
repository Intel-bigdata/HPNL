package com.intel.hpnl.api;

import java.nio.ByteBuffer;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

public class HpnlBufferAllocator {
    private final BufferCache<HpnlBuffer> CACHE_TINY;
    private final BufferCache<HpnlBuffer> CACHE_SMALL;
    private final BufferCache<HpnlBuffer> CACHE_MEDIUM;
    private final BufferCache<HpnlBuffer> CACHE_LARGE;
    private BufferCache<HpnlBuffer> CACHE_MAXSIZE;

    private BufferCache.CacheHandler<HpnlBuffer> cacheHandler;
    private int maxSize;

    public static final int BUFFER_TINY = 512;
    public static final int BUFFER_SMALL = 1024;
    public static final int BUFFER_MEDIUM = 4096;
    public static final int BUFFER_LARGE = 8192;

    private static final int BUFFER_ID_RANGES = 1000000;
    private static final int MIN_DEFAULT_BUFFER_ID = -BUFFER_ID_RANGES;
    private static int currentBufferIdLimit = MIN_DEFAULT_BUFFER_ID;
    private static Queue<IdRange> idRangeQueue = new ConcurrentLinkedQueue<>();

    public static final int NUM_UNIT = 128;

    public static final int BUFFER_METADATA_SIZE = AbstractHpnlBuffer.BASE_METADATA_SIZE;

    private static final BufferCache.CacheHandler<HpnlBuffer> DEFAULT_CACHE_HANDLER =
            new BufferCacheHandler(new IdRange(-1, MIN_DEFAULT_BUFFER_ID));

    private static final HpnlBufferAllocator DEFAULT = new HpnlBufferAllocator(DEFAULT_CACHE_HANDLER);

    private HpnlBufferAllocator(BufferCache.CacheHandler<HpnlBuffer> cacheHandler){
        this.cacheHandler = cacheHandler;
        HpnlConfig config = HpnlConfig.getInstance();
        int tinyNum = config.getBufferNumTiny();
        int smallNum = config.getBufferNumSmall();
        int mediumNum = config.getBufferNumMedium();
        int largeNum = config.getBufferNumLarge();

        CACHE_TINY = BufferCache.getInstance(cacheHandler, BUFFER_TINY, tinyNum, tinyNum);
        CACHE_SMALL = BufferCache.getInstance(cacheHandler, BUFFER_SMALL, smallNum, smallNum);
        CACHE_MEDIUM = BufferCache.getInstance(cacheHandler, BUFFER_MEDIUM, mediumNum, mediumNum);
        CACHE_LARGE = BufferCache.getInstance(cacheHandler, BUFFER_LARGE, largeNum, largeNum);
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
                int num = HpnlConfig.getInstance().getBufferNumMax();
                allocator.CACHE_MAXSIZE = BufferCache.getInstance(allocator.cacheHandler, maxSize,
                        num, num);
            }
        }
    }

    public static HpnlBufferAllocator getInstance(){
        return getInstance(false);
    }

    public static HpnlBufferAllocator getInstance(boolean largePool){
        IdRange idRange = idRangeQueue.poll();
        if(idRange == null) {
            synchronized (idRangeQueue) {
                int start = currentBufferIdLimit;
                currentBufferIdLimit -= BUFFER_ID_RANGES;
                if (currentBufferIdLimit > 0) {
                    throw new IllegalStateException("reach buffer limit: " + currentBufferIdLimit);
                }
                idRange = new IdRange(start-1, currentBufferIdLimit);
            }
        }
        return new HpnlBufferAllocator(new BufferCacheHandler(idRange));
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

    public void clear(){
        CACHE_TINY.clear();
        CACHE_SMALL.clear();
        CACHE_MEDIUM.clear();
        CACHE_LARGE.clear();
        if(CACHE_MAXSIZE != null){
            CACHE_MAXSIZE.clear();
        }
        IdRange idRange = ((BufferCacheHandler)cacheHandler).idRange;
        idRange.reset();
        idRangeQueue.offer(idRange);
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
        private IdRange idRange;

        public BufferCacheHandler(IdRange idRange){
            this.idRange = idRange;
        }
        @Override
        public HpnlBuffer newInstance(BufferCache<HpnlBuffer> cache, int size) {
            HpnlBuffer buffer = new HpnlGlobalBuffer(cache, idRange.next(), size);
            return buffer;
        }

        public IdRange getIdRange() {
            return idRange;
        }
    }

    private static class IdRange{
        private final int start;
        private final int end;
        private final AtomicInteger idGen;

        public IdRange(int start, int end){
            if(start >= 0){
                throw new IllegalArgumentException("start needs to be less than 0. "+start);
            }
            if(start < end){
                throw new IllegalArgumentException("end needs to be less than start. "+end);
            }
            this.start = start;
            this.end = end;
            idGen = new AtomicInteger(start);
        }

        public int next(){
            int id = idGen.getAndDecrement();
            if (id < end) {
                throw new IllegalArgumentException("id should not be less than limit. " + end);
            }
            return id;
        }

        public void reset(){
            idGen.set(start);
        }
    }

    public static class HpnlGlobalBuffer extends AbstractHpnlBuffer{
        private BufferCache<HpnlBuffer> cache;

        public HpnlGlobalBuffer(BufferCache<HpnlBuffer> cache, int id, int size){
            super(id);
            this.cache = cache;
            byteBuffer = ByteBuffer.allocateDirect(size);
            if(cache != null){
                cache.putBuffer(id, this);
            }
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
