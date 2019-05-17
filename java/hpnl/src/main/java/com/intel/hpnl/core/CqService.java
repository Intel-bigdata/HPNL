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

  public void addExternalEvent(int cqIndex, ExternalHandler externalHandler) {
    ((CqTask)cqTasks.get(cqIndex)).externalHandlers.add(externalHandler);
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
    private BlockingQueue<ExternalHandler> externalHandlers = new LinkedBlockingQueue<>();

    public CqTask(int index) {
      super();
      this.index = index;
      this.name = "CqTask "+index;
    }

    @Override
    public String toString(){
      return name;
    }

    @Override
    public void waitEvent() {
      if (processEvent(index) == -1) {
        log.warn("wait or process CQ event error in CQ task {}. ignoring", index);
      }
    }

    private int processEvent(int index) {
      int ret = 0;
      if (wait_cq_event(index, nativeHandle) < 0) {
        ret = -1;
      }
      int extRet = 0;
      if(!externalHandlers.isEmpty()) {
        extRet = processExternalEvent();
      }
      return ret == -1 ? ret : extRet;
    }

    @Override
    protected void cleanUp(){
      log.info("process remaining external events");
      processExternalEvent();
    }

    @Override
    protected Logger getLogger(){
      return log;
    }

    private int processExternalEvent(){
      int ret = 0;
      ExternalHandler handler = externalHandlers.poll();
      while(handler != null){
        try {
          handler.handle();
        }catch (Throwable throwable){
          throwable.printStackTrace();
          ret = -1;
        }
        handler = externalHandlers.poll();
      }
      return ret;
    }
  }
}
