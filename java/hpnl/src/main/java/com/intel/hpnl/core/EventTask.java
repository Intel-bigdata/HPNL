package com.intel.hpnl.core;

import org.slf4j.Logger;

import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Base class of CQ task and EQ task. Bound to only one thread
 */
public abstract class EventTask implements Runnable {
  private volatile CountDownLatch completed;
  private final AtomicBoolean running = new AtomicBoolean(false);

  private final Queue<TrackedTask> pendingTasks = new ConcurrentLinkedQueue<>();
  private volatile boolean lackSendBuffer = false;

  public EventTask() {
    running.set(true);
  }

  @Override
  public void run(){
    boolean failed = false;
    try {
      while (running.get()) {
        waitEvent();
        runPendingTasks();
      }
    }catch (Throwable throwable){
      getLogger().error("error occurred in event task "+this, throwable);
      failed = true;
    }
    if(!running.get() || failed) {
      try {
        getLogger().info("cleaning up before exiting event task");
        cleanUp();
      } catch (Throwable throwable) {
        getLogger().error("error occurred during clean-up in task " + this, throwable);
      }
      //for explicit stop of this task by invoking the stop method
      synchronized (this) {
        if (completed != null) {
          completed.countDown();
        }
      }
      //set running to false in case task's failure
      stop(false);
    }
  }

  protected void runPendingTasks(){
    lackSendBuffer = false; // assume buffer is available at beginning
    TrackedTask task = pendingTasks.peek();
    long tasks = 0L;
    while(task != null) {
      task.run();
      tasks++;
      if (lackSendBuffer) {
        //lack buffer, this tracked task will be reset with new runnable to send remaining data
        // the new task should be the first to run.
        break;
      }
      TrackedTask tt = pendingTasks.remove();
      if(tt != task){
        throw new IllegalStateException("task to be removed should be the same task");
      }
      if((tasks & 0x3F) == 0) {
        break;
      }
      task = pendingTasks.peek();
    }
  }

  public void addPendingTask(Runnable task){
    addPendingTask(task, false, false);
  }

  public void addPendingTask(Runnable task, boolean keepOrder, boolean lackSendBuffer){
    if(lackSendBuffer) {
      this.lackSendBuffer = true;//stop task running if lack of buffer
    }
    if(keepOrder){//replace current task with new task
      TrackedTask runningTask = TrackedTask.Tracker.getRunningTask();
      if(runningTask != null){
        runningTask.setTask(task);
        return;
      }
    }

    TrackedTask trackedTask = TrackedTask.newInstance();
    trackedTask.setTask(task);
    pendingTasks.offer(trackedTask);
  }

  public boolean isStopped(){
    return !running.get();
  }

  protected abstract void waitEvent();
  protected abstract Logger getLogger();

  protected void cleanUp(){}

  public void stop() {
    stop(true);
  }

  private void stop(boolean needWait){
    if(running.get()) {
      synchronized (this) {
        if(running.get()) {
          running.set(false);
          if(needWait) {
            completed = new CountDownLatch(1);
          }
          getLogger().info(this+" is stopped");
        }
      }
    }
  }

  public void waitToComplete() throws InterruptedException {
    if(completed == null){
      throw new IllegalStateException("invoke stop() method before waitToComplete()");
    }
    completed.await();
  }
}
