package com.intel.hpnl.api;

import org.slf4j.Logger;

public abstract class EventTask implements Runnable {
//  private volatile CountDownLatch completed;
//  private final AtomicBoolean running = new AtomicBoolean(false);

  private long startTime;

  protected static final long DEFAULT_DURATION = 100*1000000;
  protected static final int CHECK_DEADLINE_INTERVAL = 64;

  public EventTask() {
//    this.running.set(true);
  }

  @Override
  public void run() {
      try {
//        startTime = System.nanoTime();
//        processEvents(startTime + DEFAULT_DURATION);
//        long ioTime = System.nanoTime() - startTime;
//        this.runPendingTasks(ioTime * (100 - ioRatio) / ioRatio);
        if(this.waitEvent() < 0){
          this.getLogger().warn("event process with error");
        }
      } catch (Throwable th) {
        this.getLogger().error("error occurred in event task " + this, th);
        afterError();
      }
  }

  private void afterError(){
    if (this.getLogger().isDebugEnabled()) {
      this.getLogger().info("cleaning up before exiting event task");
    }
    try {
      this.cleanUp();
    } catch (Throwable var4) {
      this.getLogger().error("error occurred during clean-up in task " + this, var4);
    }
  }

//  private void processEvents(){
//
//  }

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

//  private void runPendingTasks() throws Exception{
//    Runnable task;
//    if((task = this.pendingTasks.take()) != null) {
//      task.run();
//    }
//  }

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

//  public void addPendingTask(Runnable task) throws InterruptedException {
//    this.pendingTasks.put(task);
//  }

//  public boolean isStopped() {
//    return !this.running.get();
//  }

  protected abstract int waitEvent();

  protected abstract Logger getLogger();

  protected void cleanUp() {
  }

//  public void stop() {
//    this.stop(true);
//  }
//
//  private void stop(boolean needWait) {
//    if (this.running.get()) {
//      synchronized(this) {
//        if (this.running.get()) {
//          this.running.set(false);
//          if (needWait) {
//            this.completed = new CountDownLatch(1);
//          }
//
//          this.getLogger().info(this + " is stopping");
//        }
//      }
//    }
//
//  }
//
//  public void waitToComplete() throws InterruptedException {
//    if (this.completed == null) {
//      throw new IllegalStateException("invoke stop() method before waitToComplete()");
//    } else {
//      this.completed.await();
//    }
//  }
}
