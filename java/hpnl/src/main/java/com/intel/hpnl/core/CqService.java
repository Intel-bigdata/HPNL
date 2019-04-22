package com.intel.hpnl.core;

import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.Map;
import java.util.HashMap;

public class CqService {
  public CqService(EqService service) {
    this.eqService = service;
    this.serviceNativeHandle = eqService.getNativeHandle();
    this.cqThreads = new ArrayList<CqThread>();
    this.indexMap = new HashMap<>();
    this.externalHandlers = new ArrayList<LinkedBlockingDeque<ExternalHandler>>();

    this.eqService.setCqService(this);
  }

  public CqService init() {
    if (init(serviceNativeHandle) == -1)
      return null;
    return this;
  }

  public int start() {
    int workerNum = this.eqService.getWorkerNum();
    try {
      for (int i = 0; i < workerNum; i++) {
          CqThread cqThread = new CqThread(this, i, affinities==null ? -1 : 1L<<affinities[i]);
          cqThreads.add(cqThread);
          this.indexMap.put(i, cqThread.getId());
        this.externalHandlers.add(new LinkedBlockingDeque<ExternalHandler>());
      }
    } catch (ArrayIndexOutOfBoundsException ex) {
      System.out.println("try to set thread affinity.");
    }
    for (CqThread cqThread : cqThreads) {
      cqThread.start();
    }
    return 0;
  }

  public void shutdown() {
    for (CqThread cqThread : cqThreads) {
      synchronized(CqService.class) {
        cqThread.shutdown();
      }
    }
    free(nativeHandle);
  }

  public void join() {
    try {
      for (CqThread cqThread : cqThreads) {
        cqThread.join();
      }
    } catch (InterruptedException e) {
      e.printStackTrace();
    } finally {
    }
  }

  public void setAffinities(int[] affinities) {
    this.affinities = affinities; 
  }

  private void handleCqCallback(long eq, int eventType, int bufferId, int block_buffer_size) {
    Connection connection = eqService.getCon(eq);
    if (connection != null) {
      connection.handleCallback(eventType, bufferId, block_buffer_size);
    }
  }

  public long getThreadId(int index) {
    return this.indexMap.get(index);
  }

  public void addExternalEvent(int index, ExternalHandler externalHandler) {
    this.externalHandlers.get(index).add(externalHandler);
  }

  private int waitExternalEvent(int index) {
    LinkedBlockingDeque<ExternalHandler> externalHandlerQueue = this.externalHandlers.get(index);
    if (!externalHandlerQueue.isEmpty()) {
      externalHandlerQueue.poll().handle();
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
  private long nativeHandle;
  private EqService eqService;
  private long serviceNativeHandle;
  private List<CqThread> cqThreads;
  private ArrayList<LinkedBlockingDeque<ExternalHandler>> externalHandlers;
  private int[] affinities = null;
  private Map<Integer, Long> indexMap;
}
