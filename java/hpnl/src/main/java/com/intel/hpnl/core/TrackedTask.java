package com.intel.hpnl.core;

import io.netty.util.Recycler;
import io.netty.util.concurrent.FastThreadLocal;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class TrackedTask implements Runnable {

    private Runnable preTask;

    private Runnable task;

    private Recycler.Handle<TrackedTask> handler;

    private final static Recycler<TrackedTask> RECYCLER = new Recycler<TrackedTask>() {
        @Override
        protected TrackedTask newObject(Handle<TrackedTask> handle) {
            return new TrackedTask(handle);
        }
    };

    private static final Logger log = LoggerFactory.getLogger(TrackedTask.class);

    private TrackedTask(Recycler.Handle<TrackedTask> handler){
        this.handler = handler;
    }

    protected static TrackedTask newInstance(){
        return RECYCLER.get();
    }

    /**
     * @param task
     */
    protected void setTask(Runnable task){
        synchronized (this) {//could be set by other threads, thus sync
            if (this.preTask != null) {
                throw new IllegalStateException("preTask should be null when set new task");
            }
            this.preTask = this.task;
            this.task = task;
        }
    }

    @Override
    public void run() {
        TrackedTask runningTask = Tracker.getRunningTask();
        if(runningTask != null){
            throw new IllegalStateException("there should be no running task");
        }

        Tracker.setRunningTask(this);
        try{
            Runnable taskToRun;
            synchronized (this){//same as setTask, need sync
                taskToRun = task;
            }
            if(taskToRun != null) {//check null since TrackedTask could be not removed from task queue after its done
                taskToRun.run();
            }
        }catch (Throwable th){
            log.error("failed to run task", th);
        }finally {
            Tracker.clear();
            synchronized (this) {// preTask and task could be set by other threads during execution, thus sync
                if (preTask == null) {//task is done, to be recycled
                    task = null;
                    handler.recycle(this);
                } else { // will continue task
                    preTask = null;
                }
            }
        }
    }

    protected static class Tracker{
        public static final FastThreadLocal<TrackedTask> runningTask = new FastThreadLocal<>();

        public static TrackedTask getRunningTask(){
            return runningTask.get();
        }

        public static void setRunningTask(TrackedTask task){
            runningTask.set(task);
        }

        public static void clear(){
            runningTask.remove();
        }
    }
}
