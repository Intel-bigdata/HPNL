package com.intel.hpnl.core;

import org.slf4j.Logger;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Base class of CQ task and EQ task
 */
public abstract class EventTask implements Runnable {
  protected volatile CountDownLatch completed;
  protected final AtomicBoolean running = new AtomicBoolean(false);
  protected final AtomicBoolean pause = new AtomicBoolean(false);

  public EventTask() {
    running.set(true);
  }

  @Override
  public void run(){
    boolean failed = false;
    try {
      while (running.get() && !pause.get()) {
        waitEvent();
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

  public boolean isStopped(){
    return !running.get();
  }

  /**
   * pause this event task so that other task can be run in the same thread executor.
   */
  public void pause(){
    pause.set(true);
    getLogger().info(this+" paused");
  }

  /**
   * resume this event task whe other tasks are done
   */
  public void resume(){
    pause.set(false);
    getLogger().info(this+" resumed");
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
