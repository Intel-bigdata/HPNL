package com.intel.hpnl.api;

import java.util.concurrent.ConcurrentLinkedQueue;

public class ReceiveTaskCache {
    private int size;
    private final ConcurrentLinkedQueue<CallbackTask> TASK_CACHE = new ConcurrentLinkedQueue<>();

    private static final int MAX_CACHE_SIZE = 32768;
    private static ThreadLocal<ReceiveTaskCache> threadCache = ThreadLocal.withInitial(() -> new ReceiveTaskCache());

    public static CallbackTask getInstance(){
        ReceiveTaskCache cache = threadCache.get();
        CallbackTask task = cache.TASK_CACHE.poll();
        if(task == null){
            if(cache.size < MAX_CACHE_SIZE){
                cache.size++;
                task = new CallbackTask(cache);
            }else{
                task = new CallbackTask(null);
            }
        }
        return task;
    }

    public static class CallbackTask implements Runnable {
        public Connection connection;
        public Handler handler;
        public int bufferId;
        public int bufferSize;

        private ReceiveTaskCache cache;

        private CallbackTask(ReceiveTaskCache cache){
            this.cache = cache;
        }

        @Override
        public void run() {
            try{
                handler.handle(connection, bufferId, bufferSize);
            }finally{
                if(cache != null) {
                    cache.TASK_CACHE.offer(this);
                }
            }
        }
    }
}
