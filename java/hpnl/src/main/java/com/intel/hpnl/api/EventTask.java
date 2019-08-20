package com.intel.hpnl.api;

import java.util.Queue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicBoolean;
import org.slf4j.Logger;

public abstract class EventTask implements Runnable {
  private volatile CountDownLatch completed;
  private final AtomicBoolean running = new AtomicBoolean(false);
  private final BlockingQueue<Runnable> pendingTasks;
  private final int ioRatio ;

  private long startTime;

  protected static final long DEFAULT_DURATION = 100*1000000;
  protected static final int CHECK_DEADLINE_INTERVAL = 64;

  public EventTask(int ioRatio, BlockingQueue<Runnable> queue) {
    this.ioRatio = ioRatio;
    this.pendingTasks = queue;
    this.running.set(true);
  }

  @Override
  public void run() {
    boolean failed = false;
    running.set(true);

    try {
      while(this.running.get()) {
//        startTime = System.nanoTime();
//        processEvents(startTime + DEFAULT_DURATION);
//        long ioTime = System.nanoTime() - startTime;
//        this.runPendingTasks(ioTime * (100 - ioRatio) / ioRatio);
        processEvents();
        runPendingTasks();
      }
    } catch (Throwable var6) {
      this.getLogger().error("error occurred in event task " + this, var6);
      failed = true;
    }

    if (!this.running.get() || failed) {
      try {
        if (this.getLogger().isDebugEnabled()) {
          this.getLogger().info("cleaning up before exiting event task");
        }

        this.cleanUp();
      } catch (Throwable var4) {
        this.getLogger().error("error occurred during clean-up in task " + this, var4);
      }

      synchronized(this) {
        if (this.completed != null) {
          this.completed.countDown();
        }
      }
      this.stop(false);
    }
  }

  private void processEvents(){
    this.waitEvent();
  }

//  private void processEvents(long deadline){
//    int ret = 1;
//    int cnt = 0;
//    int interval = CHECK_DEADLINE_INTERVAL;
//    while(this.running.get() && ret > 0){
//      ret = this.waitEvent();
//      cnt++;
//      if(cnt >= interval){
//        if(System.nanoTime() >= deadline){
//          return;
//        }
//        interval = interval >> 1;
//        cnt = 0;
//      }
//    }
//  }

  private void runPendingTasks() throws Exception{
    Runnable task;
    if((task = this.pendingTasks.take()) != null) {
      task.run();
    }
  }

//  protected void runPendingTasks(long timeout) {
//    long deadline = System.nanoTime() + timeout;
//    Runnable task;
//    int taskCnt = 0;
//    int interval = CHECK_DEADLINE_INTERVAL;
//    while(this.running.get() && ((task = this.pendingTasks.poll()) != null)){
//      task.run();
//      taskCnt++;
//      if(taskCnt >= interval){
//        if(System.nanoTime() >= deadline){
//          return;
//        }
//        taskCnt = 0;
//        interval = interval >> 1;
//      }
//    }
//  }

  public void addPendingTask(Runnable task) throws InterruptedException {
    this.pendingTasks.put(task);
  }

  public boolean isStopped() {
    return !this.running.get();
  }

  protected abstract int waitEvent();

  protected abstract Logger getLogger();

  protected void cleanUp() {
  }

  public void stop() {
    this.stop(true);
  }

  private void stop(boolean needWait) {
    if (this.running.get()) {
      synchronized(this) {
        if (this.running.get()) {
          this.running.set(false);
          if (needWait) {
            this.completed = new CountDownLatch(1);
          }

          this.getLogger().info(this + " is stopping");
        }
      }
    }

  }

  public void waitToComplete() throws InterruptedException {
    if (this.completed == null) {
      throw new IllegalStateException("invoke stop() method before waitToComplete()");
    } else {
      this.completed.await();
    }
  }
}
