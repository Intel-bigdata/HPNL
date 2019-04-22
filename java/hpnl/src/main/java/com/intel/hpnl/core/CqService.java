package com.intel.hpnl.core;

import java.util.List;
import java.util.ArrayList;

public class CqService {
  private long nativeHandle;
  private EqService eqService;
  private long serviceNativeHandle;
  private List<EventTask> cqTasks;
  private List<ExternalHandler> externalHandlers;

  static {
    System.loadLibrary("hpnl");
  }

  public CqService(EqService service, long serviceNativeHandle) {
    this.eqService = service;
    this.serviceNativeHandle = serviceNativeHandle;

    this.cqTasks = new ArrayList();
    int workerNum = this.eqService.getWorkerNum();
    for (int i = 0; i < workerNum; i++) {
      EventTask task = new CqTask(i);
      cqTasks.add(task);
    }

    this.externalHandlers = new ArrayList();
  }

  public CqService init() {
    if (init(serviceNativeHandle) == -1)
      return null;
    return this;
  }

  public List<EventTask> getEventTasks() {
    return cqTasks;
  }

  public void stop() {
    synchronized (this) {
      for (EventTask task : cqTasks) {
        task.stop();
      }
      waitToComplete();
    }
  }

  private void waitToComplete() {
    try {
      for (EventTask task : cqTasks) {
        task.waitToComplete();
      }
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  public void addExternalEvent(ExternalHandler externalHandler) {
    externalHandlers.add(externalHandler); 
  }

  private int waitExternalEvent(int index) {
    for (ExternalHandler handler: externalHandlers) {
      handler.handle();
    }
    return 0;
  }

  public int wait_event(int index) {
    if (wait_cq_event(index, nativeHandle) < 0) {
      return -1;
    }
    waitExternalEvent(index);
    return 0;
  }

  public native int wait_cq_event(int index, long nativeHandle);
  private native int init(long Service);
  public native void finalize();
  private native void free(long nativeHandle);


  public class CqTask extends EventTask {
    private int index;

    public CqTask(int index) {
      super();
      this.index = index;
    }

    @Override
    public void loopEvent() {
      while (running.get()) {
        if (wait_event(index) == -1) {
          stop();
        }
      }
    }
  }
}
