package com.intel.hpnl.core;

import com.intel.hpnl.api.*;

import java.nio.ByteBuffer;

public class FixedSizeBufferAllocator implements HpnlBufferAllocator {
    private final BufferCache<HpnlBuffer> smallCache;
    private final BufferCache<HpnlBuffer> largeCache;
    private final BufferCache<HpnlBuffer> maxCache;

    private BufferCache.CacheHandler<HpnlBuffer> cacheHandler;

    private int smallCap;
    private int largeCap;
    private int maxCap;

    private String role;

    private static final BufferIdManager idManager = new BufferIdManager();

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
        this.role = role;
        maxCache = BufferCache.getInstance(cacheHandler, maxCap, maxNum);
    }


    public static FixedSizeBufferAllocator getInstance(String role, int maxCap){
        BufferIdManager.IdRange idRange = idManager.getUniqueIdRange(role);
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
        BufferIdManager.IdRange idRange = ((BufferCacheHandler)cacheHandler).idRange;
        idManager.reclaim(role, idRange);
    }

    private static class BufferCacheHandler implements BufferCache.CacheHandler<HpnlBuffer>{
        private BufferIdManager.IdRange idRange;

        public BufferCacheHandler(BufferIdManager.IdRange idRange){
            this.idRange = idRange;
        }
        @Override
        public HpnlBuffer newInstance(BufferCache<HpnlBuffer> cache, int size) {
            HpnlBuffer buffer = new HpnlGlobalBuffer(cache, idRange.next(), size);
            return buffer;
        }

        public BufferIdManager.IdRange getIdRange() {
            return idRange;
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

