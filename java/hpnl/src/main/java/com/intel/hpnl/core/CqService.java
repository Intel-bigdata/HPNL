package com.intel.hpnl.core;

import com.intel.hpnl.api.EventTask;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class CqService {
  private long nativeHandle;
  private EqService eqService;
  private long serviceNativeHandle;
  private List<EventTask> cqTasks;
  private static final Logger log = LoggerFactory.getLogger(CqService.class);

  public CqService(EqService service, long serviceNativeHandle) {
    this.eqService = service;
    this.serviceNativeHandle = serviceNativeHandle;
    this.cqTasks = new ArrayList();
    int workerNum = this.eqService.getWorkerNum();

    for(int i = 0; i < workerNum; ++i) {
      EventTask task = new CqService.CqTask(i);
      this.cqTasks.add(task);
    }

    this.eqService.setCqService(this);
  }

  public CqService init() {
    return this.init(this.serviceNativeHandle) == -1 ? null : this;
  }

  public List<EventTask> getEventTasks() {
    return this.cqTasks;
  }

  public void addExternalEvent(int cqIndex, Runnable task) {
    ((EventTask)this.cqTasks.get(cqIndex)).addPendingTask(task);
  }

  public native int wait_cq_event(int var1, long var2);

  private native int init(long var1);

  public native void finalize();

  private native void free(long var1);

  public void stop() {
    Iterator var1 = this.cqTasks.iterator();

    while(var1.hasNext()) {
      EventTask task = (EventTask)var1.next();
      task.stop();
    }

    this.waitToComplete();
    this.free(this.nativeHandle);
  }

  private void waitToComplete() {
    try {
      Iterator var1 = this.cqTasks.iterator();

      while(var1.hasNext()) {
        EventTask task = (EventTask)var1.next();
        task.waitToComplete();
      }
    } catch (InterruptedException var3) {
      log.error("CQ task is interrupted when wait its completion", var3);
    }

  }

  public class CqTask extends EventTask {
    private int index;
    private String name;

    public CqTask(int index) {
      this.index = index;
      this.name = "CqTask " + index;
    }

    public String toString() {
      return this.name;
    }

    public void waitEvent() {
      if (CqService.this.wait_cq_event(this.index, CqService.this.nativeHandle) < 0) {
        CqService.log.warn("wait or process CQ event error in CQ task {}. ignoring", this.index);
      }

    }

    protected void cleanUp() {
    }

    protected Logger getLogger() {
      return CqService.log;
    }
  }
}
