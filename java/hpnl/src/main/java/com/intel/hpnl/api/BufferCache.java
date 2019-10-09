package com.intel.hpnl.api;

import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class BufferCache<T> {
    private int count;
    private int size;
    private final int bufferLimit;
    private CacheHandler<T> handler;
    private Queue<T> pool;

    private Map<Integer, T> bufferMap;
    private ThreadLocal<BufferCache<T>> threadLocal;

    private BufferCache(CacheHandler<T> handler, int size, int bufferLimit){
        this.handler = handler;
        this.bufferLimit = bufferLimit;
        this.size = size;
    }

    public static <T> BufferCache<T> getInstance(CacheHandler<T> handler, int size,
                                                 int globalBufferLimit, int threadBufferLimit){
        BufferCache<T> cache = new BufferCache<>(handler, size, globalBufferLimit);
        cache.pool = new ConcurrentLinkedQueue<>();
        cache.bufferMap = new ConcurrentHashMap<>();
        cache.threadLocal = ThreadLocal.withInitial(() -> {
            BufferCache<T> tCache = new BufferCache<>(handler, size, threadBufferLimit);
            tCache.pool = new ConcurrentLinkedQueue<>();
            tCache.bufferMap = new ConcurrentHashMap<>();
            return tCache;
        });
        return cache;
    }

    public T getInstance(){
        BufferCache<T> cache = threadLocal.get();
        //get from thread local first
        T object = getFromCache(cache, size);
        if(object != null){
            return object;
        }
        //get from global
//        object = getFromCache(this, size, false);
//        if(object != null){
//            return object;
//        }
        //not for cache
        return handler.newInstance(null, size);
    }

    private T getFromCache(BufferCache<T> cache, int size){
        T object = cache.pool.poll();
        if(object != null){
            return object;
        }
        if (cache.count >= cache.bufferLimit) {
            return null;
        }
        cache.count++;

        return handler.newInstance(cache, size);
    }

    public void reclaim(T object){
        pool.offer(object);
    }

    public void putBuffer(int id, T object){
        bufferMap.put(Integer.valueOf(id), object);
    }

    public T getBuffer(int id){
        return bufferMap.get(id);
    }

    public void clear(){
        pool.clear();
        bufferMap.clear();
        threadLocal.get().clear();
    }

    public interface CacheHandler<T>{
        T newInstance(BufferCache<T> cache, int size);
    }
}
