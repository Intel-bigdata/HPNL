package com.intel.hpnl.api;

import io.netty.util.concurrent.FastThreadLocal;

import java.lang.ref.WeakReference;
import java.util.Arrays;
import java.util.Map;
import java.util.WeakHashMap;
import java.util.concurrent.atomic.AtomicInteger;

import static java.lang.Math.min;

public class BufferCache<T> {
    private int index;
    private int count;
    private final int size;
    private final int bufferLimit;
    private final CacheHandler<T> handler;
    private T pool[];

    private ThreadLocalBufferCache<T> threadLocal;
    private Thread thread;

    private WeakOrderQueue cursor, prev;
    private volatile WeakOrderQueue head;

    private static final int POOL_INITIAL_SIZE = 64;

    private BufferCache(CacheHandler<T> handler, int size, int bufferLimit){
        this.handler = handler;
        this.bufferLimit = bufferLimit;
        this.size = size;
        this.index = -1;
    }

    public static <T> BufferCache<T> getInstance(CacheHandler<T> handler, int size, int threadBufferLimit){
        BufferCache<T> cache = new BufferCache<>(handler, size, threadBufferLimit);
        cache.threadLocal = new ThreadLocalBufferCache(handler, size, threadBufferLimit);
        return cache;
    }

    public T getInstance(){
        BufferCache<T> cache = threadLocal.get();
        //get from thread local first
        T object = getFromThreadLocalCache(cache, size);
        if(object != null){
            return object;
        }
        //not for cache
        return handler.newInstance(null, size);
    }

    private T getFromThreadLocalCache(BufferCache<T> cache, int size){
        if(index >= 0){
            return pool[index--];
        }
        if(scavenge()){
            return pool[index--];
        }
        if (cache.count >= cache.bufferLimit) {
            return null;
        }
        cache.count++;

        return handler.newInstance(cache, size);
    }

    // Marked as synchronized to ensure this is serialized.
    synchronized void setHead(WeakOrderQueue queue) {
        queue.setNext(head);
        head = queue;
    }

    private boolean scavenge() {
        WeakOrderQueue prev;
        WeakOrderQueue cursor = this.cursor;
        if (cursor == null) {
            prev = null;
            cursor = head;
            if (cursor == null) {
                return false;
            }
        } else {
            prev = this.prev;
        }

        boolean success = false;
        do {
            if (cursor.transfer(this)) {
                success = true;
                break;
            }
            WeakOrderQueue next = cursor.next;
            if (cursor.owner.get() == null) {
                // If the thread associated with the queue is gone, unlink it, after
                // performing a volatile read to confirm there is no data left to collect.
                // We never unlink the first queue, as we don't want to synchronize on updating the head.
                if (cursor.hasFinalData()) {
                    for (;;) {
                        if (cursor.transfer(this)) {
                            success = true;
                        } else {
                            break;
                        }
                    }
                }

                if (prev != null) {
                    prev.setNext(next);
                }
            } else {
                prev = cursor;
            }

            cursor = next;

        } while (cursor != null && !success);

        this.prev = prev;
        this.cursor = cursor;
        return success;
    }

    public void reclaim(T object){
        Thread currentThread = Thread.currentThread();
        if (thread == currentThread) {
            pushNow(object);
        } else {
            pushLater(object, currentThread);
        }
    }

    private void pushLater(T object, Thread currentThread) {
        Map<BufferCache<?>, WeakOrderQueue<?>> delayedRecycled = DELAYED_RECYCLED.get();
        WeakOrderQueue<T> queue = (WeakOrderQueue<T>) delayedRecycled.get(this);
        if (queue == null) {
            queue = WeakOrderQueue.newQueue(this, thread);
            delayedRecycled.put(this, queue);
        }
        queue.add(object);
    }

    private void pushNow(T object) {
        if(remaining() > 0){
            pool[++index] = object;
            return;
        }
        if(pool.length == bufferLimit){
            throw new IllegalStateException("buffer capacity reach limit, "+bufferLimit);
        }
        increaseCapacity(pool.length+1);
        pool[++index] = object;
    }

    private int increaseCapacity(int expected){
        if(bufferLimit == pool.length){
            return bufferLimit;
        }
        int limit = min(expected, bufferLimit);
        int newLen = pool.length << 1;
        while(newLen < limit){
            newLen = newLen << 1;
        }
        pool = Arrays.copyOf(pool, min(newLen, bufferLimit));
        return pool.length;
    }

    private int remaining(){
        return pool.length - index - 1;
    }

    public void clear(){
        pool = null;
        if(threadLocal != null) {
            threadLocal.remove();
        }
    }

    private static final FastThreadLocal<Map<BufferCache<?>, WeakOrderQueue<?>>> DELAYED_RECYCLED =
        new FastThreadLocal<Map<BufferCache<?>, WeakOrderQueue<?>>>() {
            @Override
            protected Map<BufferCache<?>, WeakOrderQueue<?>> initialValue() {
                return new WeakHashMap();
            }
        };

    private static final class ThreadLocalBufferCache<T> extends FastThreadLocal<BufferCache<T>>{

        private final CacheHandler<T> handler;
        private final int size;
        private final int threadBufferLimit;

        public ThreadLocalBufferCache(CacheHandler<T> handler, int size, int threadBufferLimit){
            this.handler = handler;
            this.size = size;
            this.threadBufferLimit = threadBufferLimit;
        }

        @Override
        protected synchronized BufferCache<T> initialValue() {
            BufferCache<T> tCache = new BufferCache<>(handler, size, threadBufferLimit);
            tCache.pool = (T[])new Object[POOL_INITIAL_SIZE];
            tCache.thread = Thread.currentThread();
            return tCache;
        }

        @Override
        protected void onRemoval(BufferCache<T> bufferCache) {
            bufferCache.clear();
        }
    }

    private static final class WeakOrderQueue<T> {

        private static final int LINK_CAPACITY = 128;

        // Let Link extend AtomicInteger for intrinsics. The Link itself will be used as writerIndex.
        @SuppressWarnings("serial")
        private static final class Link<T> extends AtomicInteger {
            private final T[] elements = (T[])new Object[LINK_CAPACITY];;
            private int readIndex;
            private Link<T> next;
        }

        // chain of data items
        private Link<T> head, tail;
        // pointer to another queue of delayed items for the same stack
        private WeakOrderQueue<T> next;
        private final WeakReference<Thread> owner;

        private WeakOrderQueue() {
            owner = null;
        }

        private WeakOrderQueue(BufferCache<T> cache, Thread thread) {
            head = tail = new Link();
            owner = new WeakReference<>(thread);
        }

        static <T> WeakOrderQueue newQueue(BufferCache<T> cache, Thread thread) {
            WeakOrderQueue queue = new WeakOrderQueue(cache, thread);
            // Done outside of the constructor to ensure WeakOrderQueue.this does not escape the constructor and so
            // may be accessed while its still constructed.
            cache.setHead(queue);
            return queue;
        }

        private void setNext(WeakOrderQueue<T> next) {
            assert next != this;
            this.next = next;
        }

        void add(T buffer) {
            Link<T> tail = this.tail;
            int writeIndex;
            if ((writeIndex = tail.get()) == LINK_CAPACITY) {
                this.tail = tail = tail.next = new Link();
                writeIndex = tail.get();
            }
            tail.elements[writeIndex] = buffer;
            tail.lazySet(writeIndex + 1);
        }

        boolean hasFinalData() {
            return tail.readIndex != tail.get();
        }

        // transfer as many items as we can from this queue to the BufferCache, returning true if any were transferred
        @SuppressWarnings("rawtypes")
        boolean transfer(BufferCache<T> dst) {
            Link<T> head = this.head;
            if (head == null) {
                return false;
            }

            if (head.readIndex == LINK_CAPACITY) {
                if (head.next == null) {
                    return false;
                }
                this.head = head = head.next;
            }

            final int srcStart = head.readIndex;
            int srcEnd = head.get();
            final int srcSize = srcEnd - srcStart;
            if (srcSize == 0) {
                return false;
            }

            if (srcSize > dst.remaining()) {
                int expected = srcSize + dst.index + 1; //index is -1 for empty buffer cache
                int actual = dst.increaseCapacity(expected);
                if(actual < expected){
                    throw new IllegalStateException(String.format("expected %d, actual %d", expected, actual));
                }
            }

            final T[] elements = head.elements;
            final T[] dstElems = dst.pool;
            System.arraycopy(elements, srcStart, dstElems, dst.index+1, srcSize);

            if (srcEnd == LINK_CAPACITY && head.next != null) {
                this.head = head.next;
            }
            return true;
        }
    }

    public interface CacheHandler<T>{
        T newInstance(BufferCache<T> cache, int size);
    }
}
