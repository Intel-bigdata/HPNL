package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlBuffer;
import com.intel.hpnl.api.HpnlBufferAllocator;
import com.intel.hpnl.api.HpnlConfig;
import io.netty.buffer.PooledByteBufAllocator;

public class DynamicSizeBufferAllocator implements HpnlBufferAllocator {

    private final PooledByteBufAllocator allocator;

    public static HpnlBufferAllocator getInstance(String role, int maxCap, int cores) {
        HpnlConfig config = HpnlConfig.getInstance();
        int tinyCacheSize = config.getBufferDynNumTiny(role);
        int smallCacheSize = config.getBufferDynNumSmall(role);
        int normalCacheSize = config.getBufferDynNumNormal(role);

        int directArena = cores;
        if(directArena <= 0){
            directArena = config.getBufferDynNumArena(role);
            if(directArena <= 0){
                directArena = Runtime.getRuntime().availableProcessors()*2;
            }
        }
        PooledByteBufAllocator allocator = new PooledByteBufAllocator(true, 0, directArena,
                8192, 11, tinyCacheSize, smallCacheSize, normalCacheSize, true);
        return new DynamicSizeBufferAllocator(allocator);
    }

    public DynamicSizeBufferAllocator(PooledByteBufAllocator allocator) {
        this.allocator = allocator;
    }

    @Override
    public HpnlBuffer getBuffer(int size) {
        return new HpnlNettyBuffer(allocator.directBuffer(size));
    }

    @Override
    public HpnlBuffer getBuffer(boolean large) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void clear() {}
}
