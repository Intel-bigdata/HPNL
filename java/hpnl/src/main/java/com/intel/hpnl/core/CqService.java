package com.intel.hpnl.core;

import java.util.List;
import java.util.ArrayList;

public class CqService {
  static {
    System.load("/usr/local/lib/libhpnl.so");
  }

  public CqService(EqService service, int num, long serviceNativeHandle) {
    this.eqService = service;
    this.num = num;
    this.serviceNativeHandle = serviceNativeHandle;

    Runtime.getRuntime().addShutdownHook(new CqShutdownThread(eqService, this));

    init(serviceNativeHandle);
    cqThreads = new ArrayList<CqThread>();
    for (int i = 0; i < this.num; i++) {
      CqThread cqThread = new CqThread(this, i);
      cqThreads.add(cqThread);
    }
    this.externalHandlers = new ArrayList<ExternalHandler>();
  }
  public void start() {
    for (CqThread cqThread : cqThreads) {
      cqThread.start();
    }
  }
  public void shutdown() {
    for (CqThread cqThread : cqThreads) {
      synchronized(this) {
        cqThread.shutdown();
      }
    }
  }

  public void join() {
    try {
      for (CqThread cqThread : cqThreads) {
        cqThread.join();
      }
    } catch (InterruptedException e) {
      e.printStackTrace();
    } finally {
      synchronized(this) {
        free();
      }
    }
  }

  private void handleCqCallback(long eq, int eventType, int rdma_buffer_id, int block_buffer_size) {
    Connection connection = eqService.getConMap().get(eq);
    connection.handleCallback(eventType, rdma_buffer_id, block_buffer_size);
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
    wait_cq_event(index);
    waitExternalEvent(index);
    return 0;
  }

  public native int wait_cq_event(int index);
  private native void init(long Service);
  public native void finalize();
  private native void free();
  private long nativeHandle;
  private EqService eqService;
  private int num;
  private long serviceNativeHandle;
  private List<CqThread> cqThreads;
  private List<ExternalHandler> externalHandlers;
}
