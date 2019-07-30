// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

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
    this.cqThreads = new ArrayList<>();
    this.indexMap = new HashMap<>();
    this.externalHandlers = new ArrayList<>();

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
        this.externalHandlers.add(new LinkedBlockingDeque<>());
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
    Connection connection = eqService.getConnection(eq);
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

  public void free() {
    synchronized(CqService.class) {
      free(this.nativeHandle);
    }
  }

  private native int wait_cq_event(int index, long nativeHandle);
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
