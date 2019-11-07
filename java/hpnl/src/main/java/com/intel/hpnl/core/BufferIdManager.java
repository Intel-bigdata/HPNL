package com.intel.hpnl.core;

import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

public final class BufferIdManager {

    private final Map<String, InternalIdManager> roleIdMgrMap = new ConcurrentHashMap<>();

    private final InternalIdManager defaultIdMgr = new InternalIdManager();

    public IdRange getUniqueIdRange(String role) {
        if(role == null){
            return defaultIdMgr.getUniqueIdRange();
        }
        InternalIdManager mgr = roleIdMgrMap.get(role);
        if(mgr == null) {
            mgr = new InternalIdManager();
            mgr = roleIdMgrMap.putIfAbsent(role, mgr);
        }
        return mgr.getUniqueIdRange();
    }

    public void reclaim(String role, IdRange idRange) {
        if(role == null){
            defaultIdMgr.reclaim(idRange);
            return;
        }
        roleIdMgrMap.get(role).reclaim(idRange);
    }

    private static class InternalIdManager {
        private final int BUFFER_ID_RANGES = 1000000;
        private final int BUFFER_ID_START = 0;
        private int currentBufferIdLimit = BUFFER_ID_START;
        private final Queue<IdRange> idRangeQueue = new ConcurrentLinkedQueue<>();

        public void reclaim(IdRange idRange) {
            idRange.reset();
            idRangeQueue.offer(idRange);
        }

        public IdRange getUniqueIdRange() {
            IdRange idRange = idRangeQueue.poll();
            if (idRange == null) {
                synchronized (idRangeQueue) {
                    int start = currentBufferIdLimit;
                    currentBufferIdLimit -= BUFFER_ID_RANGES;
                    if (currentBufferIdLimit > 0) {
                        throw new IllegalStateException("reach buffer limit: " + currentBufferIdLimit);
                    }
                    idRange = new BufferIdManager.IdRange(start - 1, currentBufferIdLimit);
                }
            }
            return idRange;
        }
    }

    public static class IdRange{
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
}
