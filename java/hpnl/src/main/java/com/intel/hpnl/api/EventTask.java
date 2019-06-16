package com.intel.hpnl.api;

import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;
import org.slf4j.Logger;

public abstract class EventTask implements Runnable {
  private volatile CountDownLatch completed;
  private final AtomicBoolean running = new AtomicBoolean(false);
  private final Queue<Runnable> pendingTasks = new ConcurrentLinkedQueue();

  public EventTask() {
    this.running.set(true);
  }

  public void run() {
    boolean failed = false;

    try {
      while(this.running.get()) {
        this.waitEvent();
        this.runPendingTasks();
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

  protected void runPendingTasks() {
    Runnable task = (Runnable)this.pendingTasks.poll();

    for(long tasks = 0L; task != null; task = (Runnable)this.pendingTasks.poll()) {
      task.run();
      ++tasks;
      if ((tasks & 63L) == 0L) {
        break;
      }
    }

  }

  public void addPendingTask(Runnable task) {
    this.pendingTasks.offer(task);
  }

  public boolean isStopped() {
    return !this.running.get();
  }

  protected abstract void waitEvent();

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
