package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlBufferAllocator;
import com.intel.hpnl.api.HpnlConfig;
import com.intel.hpnl.api.HpnlRdmaBuffer;
import io.netty.buffer.PooledByteBufAllocator;

public class DynamicSizeBufferAllocator implements HpnlBufferAllocator {

    private final PooledByteBufAllocator allocator;
    private final int maxCap;
    private final String role;
    private final BufferIdManager.IdRange idRange;

    private static BufferIdManager idManager = new BufferIdManager();

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
        return new DynamicSizeBufferAllocator(allocator, maxCap, role, idManager.getUniqueIdRange(role));
    }

    public DynamicSizeBufferAllocator(PooledByteBufAllocator allocator, int maxCap,
                                      String role, BufferIdManager.IdRange idRange) {
        this.allocator = allocator;
        this.maxCap = maxCap;
        this.role = role;
        this.idRange = idRange;
    }

    @Override
    public HpnlRdmaBuffer getBuffer(int size) {
        return new HpnlNettyBuffer(idRange.next(), allocator.directBuffer(size));
    }

    @Override
    public HpnlRdmaBuffer getBuffer(boolean large) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void clear() {
        idManager.reclaim(role, idRange);
    }
}
