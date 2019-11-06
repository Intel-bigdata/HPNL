package com.intel.hpnl.core;

import com.intel.hpnl.api.*;

import java.nio.ByteBuffer;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

public class FixedSizeBufferAllocator implements HpnlBufferAllocator {
    private final BufferCache<HpnlBuffer> smallCache;
    private final BufferCache<HpnlBuffer> largeCache;
    private final BufferCache<HpnlBuffer> maxCache;

    private BufferCache.CacheHandler<HpnlBuffer> cacheHandler;

    private int smallCap;
    private int largeCap;
    private int maxCap;

    private static final int BUFFER_ID_RANGES = 1000000;
    private static final int BUFFER_ID_START = 0;
    private static int currentBufferIdLimit = BUFFER_ID_START;
    private static final Queue<IdRange> idRangeQueue = new ConcurrentLinkedQueue<>();

    private FixedSizeBufferAllocator(BufferCache.CacheHandler<HpnlBuffer> cacheHandler, String role, int maxCap){
        this.cacheHandler = cacheHandler;
        HpnlConfig config = HpnlConfig.getInstance();
        int smallNum = config.getBufferNumSmall(role);
        int largeNum = config.getBufferNumLarge(role);
        int maxNum = config.getBufferNumMax(role);

        this.smallCap = config.getBufferSmallCap(role);
        this.largeCap = config.getBufferLargeCap(role);

        smallCache = BufferCache.getInstance(cacheHandler, smallCap, smallNum);
        largeCache = BufferCache.getInstance(cacheHandler, largeCap, largeNum);
        if (largeCap > maxCap) {
            throw new IllegalArgumentException(
                    String.format("max buffer size %d should be no less than large buffer size %d",
                            maxCap, largeCap));
        }
        this.maxCap = maxCap;
        maxCache = BufferCache.getInstance(cacheHandler, maxCap, maxNum);
    }


    public static FixedSizeBufferAllocator getInstance(String role, int maxCap){
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
        return new FixedSizeBufferAllocator(new BufferCacheHandler(idRange), role, maxCap);
    }

    /**
     *      * different capacities (4096, 16384, maxSize) in threadlocal
     *           * @return
     *
     */
    @Override
    public HpnlBuffer getBuffer(int cap){
        if(cap <= smallCap){
            HpnlBuffer buffer = smallCache.getInstance();
            buffer.clear();
            return buffer;
        }
        if(cap <= largeCap){
            HpnlBuffer buffer = largeCache.getInstance();
            buffer.clear();
            return buffer;
        }
        if(cap <= maxCap){
            HpnlBuffer buffer = maxCache.getInstance();
            buffer.clear();
            return buffer;
        }
        throw new IllegalArgumentException("buffer size cannot exceed "+maxCap);
    }

    @Override
    public HpnlBuffer getBuffer(boolean large){
        if(!large){
            HpnlBuffer buffer = smallCache.getInstance();
            buffer.clear();
            return buffer;
        }
        HpnlBuffer buffer = largeCache.getInstance();
        buffer.clear();
        return buffer;
    }

    @Override
    public void clear(){
        smallCache.clear();
        largeCache.clear();
        maxCache.clear();
        IdRange idRange = ((BufferCacheHandler)cacheHandler).idRange;
        idRange.reset();
        idRangeQueue.offer(idRange);
    }

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

