package com.intel.hpnl.core;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;

public abstract class EventTask implements Runnable {
  protected volatile CountDownLatch completed;
  protected final AtomicBoolean running = new AtomicBoolean(false);

  public EventTask() {
    running.set(true);
  }

  @Override
  public void run(){
    try {
      while (running.get()) {
        waitEvent();
      }
    }catch (Throwable throwable){
      System.err.println("error occurred in event task "+this.getClass().getName());
      throwable.printStackTrace();
    }
    try {
      cleanUp();
    }catch (Throwable throwable){
      System.err.println("error occurred during clean-up in task "+this.getClass().getName());
      throwable.printStackTrace();
    }
    synchronized (this) {
      if (completed != null) {
        completed.countDown();
      }
    }
  }

  protected abstract void waitEvent();

  protected void cleanUp(){}

  public void stop() {
    if(running.get()) {
      synchronized (this) {
        if(running.get()) {
          running.set(false);
          completed = new CountDownLatch(1);
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
