package com.intel.hpnl.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * A service for completion queue
 */
public class CqService {
  private long nativeHandle;
  private EqService eqService;
  private long serviceNativeHandle;
  private List<EventTask> cqTasks;

  private static final Logger log = LoggerFactory.getLogger(CqService.class);

  /**
   * construct CQ service with EqService.
   * One {@link CqTask} per completion queue
   * Number of completion queue is determined by number of worker
   * @param service
   * @param serviceNativeHandle
   */
  public CqService(EqService service, long serviceNativeHandle) {
    this.eqService = service;
    this.serviceNativeHandle = serviceNativeHandle;

    this.cqTasks = new ArrayList();
    int workerNum = this.eqService.getWorkerNum();
    for (int i = 0; i < workerNum; i++) {
      EventTask task = new CqTask(i);
      cqTasks.add(task);
    }
    this.eqService.setCqService(this);
  }

  /**
   * initialize native resources
   * @return
   */
  public CqService init() {
    if (init(serviceNativeHandle) == -1)
      return null;
    return this;
  }

  public List<EventTask> getEventTasks() {
    return cqTasks;
  }

  public void addExternalEvent(int cqIndex, Runnable task) {
    cqTasks.get(cqIndex).addPendingTask(task);
  }

  //native methods
  public native int wait_cq_event(int index, long nativeHandle);
  private native int init(long Service);
  public native void finalize();
  private native void free(long nativeHandle);

  /**
   * stop CQ service by,
   * - stop all CQ tasks and wait their completion
   * - free native resources
   */
  public void stop() {
    for (EventTask task : cqTasks) {
      task.stop();
    }
    waitToComplete();
    //delete service after all tasks completed
    free(nativeHandle);
  }

  private void waitToComplete() {
    try {
      for (EventTask task : cqTasks) {
        task.waitToComplete();
      }
    } catch (InterruptedException e) {
      log.error("CQ task is interrupted when wait its completion", e);
    }
  }

  public class CqTask extends EventTask {
    private int index;
    private String name;

    public CqTask(int index) {
      this.index = index;
      this.name = "CqTask "+index;
    }

    @Override
    public String toString(){
      return name;
    }

    @Override
    public void waitEvent() {
      if (wait_cq_event(index, nativeHandle) < 0) {
        log.warn("wait or process CQ event error in CQ task {}. ignoring", index);
      }
    }

    @Override
    protected void cleanUp(){}

    @Override
    protected Logger getLogger(){
      return log;
    }
  }
}
