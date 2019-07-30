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

import java.util.HashMap;
import java.nio.ByteBuffer;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;

public class EqService implements MemoryService {
  static {
    System.loadLibrary("hpnl");
  }

  public EqService(int worker_num, int buffer_num, boolean is_server) {
    this.worker_num = worker_num;
    this.buffer_num = buffer_num;
    this.is_server = is_server;
    this.curCon = null;
    this.rmaBufferId = new AtomicInteger(0);

    this.conMap = new HashMap<Long, Connection>();
    this.reapCons = new LinkedBlockingQueue<Connection>();
    this.rmaBufferMap = new ConcurrentHashMap<Integer, ByteBuffer>();
    this.connectLatchMap = new ConcurrentHashMap<Long, CountDownLatch>();
  }

  public EqService init() {
    if (init(worker_num, buffer_num, is_server) == -1)
      return null;
    this.eqThread = new EqThread(this);
    this.eqThread.start();
    return this;
  }

  public Connection connect(String ip, String port, long timeout) {
    synchronized (EqService.class) {
      localEq = native_connect(ip, port, nativeHandle);
      if (localEq == -1) {
        return null;
      }
      connectLatchMap.put(localEq, new CountDownLatch(1));
      add_eq_event(localEq, nativeHandle);
      try {
        CountDownLatch latch = this.connectLatchMap.get(localEq);
        assert(latch != null);
        if (timeout == 0) {
          latch.await();
          return curCon;
        } else {
          if (!latch.await(timeout, TimeUnit.MILLISECONDS)) {
            return null;
          }
        }
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }
    return null;
  }

  public int listen(String ip, String port) {
    localEq = native_connect(ip, port, nativeHandle);
    if (localEq == -1) {
      return -1;
    }
    add_eq_event(localEq, nativeHandle);
    return 0;
  }

  public void shutdown() {
    for (long key : conMap.keySet()) {
      addReapConnection(conMap.get(key));
    }
    synchronized(EqService.class) {
      eqThread.shutdown();
    }
    delete_eq_event(localEq);
  }

  public void join() {
    try {
      eqThread.join();
    } catch (InterruptedException e) {
      e.printStackTrace();
    } finally {
    }
  }

  private void establishConnection(long eq, long con, int index, String dest_addr, int dest_port, String src_addr, int src_port) {
    Connection connection = new Connection(eq, con, index, cqService.getThreadId(index), this, this.cqService);
    connection.setAddrInfo(dest_addr, dest_port, src_addr, src_port);
    conMap.put(eq, connection);
  }

  public void closeConnection(long eq) {
    Connection con = conMap.get(eq);
    if (con != null) {
      con.delCon();
      conMap.remove(eq);
    }
  }

  private void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = conMap.get(eq);
    if (connection == null) {
      throw new NullPointerException("connection is NULL when handle " + eventType + " event.");
    }
      
    if (eventType == EventType.CONNECTED_EVENT) {
      connection.setConnectedCallback(connectedCallback);
      connection.setRecvCallback(recvCallback);
      connection.setSendCallback(sendCallback);
      connection.setReadCallback(readCallback);
      connection.setShutdownCallback(shutdownCallback);
    }
    connection.handleCallback(eventType, 0, 0);
    if (!is_server && eventType == EventType.CONNECTED_EVENT) {
      curCon = connection;
      CountDownLatch latch = this.connectLatchMap.get(eq);
      if (latch == null) {
        throw new NullPointerException("connection is NULL when handle " + eventType + " event.");
      }
      latch.countDown();
      this.connectLatchMap.remove(eq);
    }
  }

  public void setConnectedCallback(Handler callback) {
    connectedCallback = callback;
  }

  public void setRecvCallback(Handler callback) {
    recvCallback = callback;
  }

  public void setSendCallback(Handler callback) {
    sendCallback = callback;
  }

  public void setReadCallback(Handler callback) {
    readCallback = callback; 
  }

  public void setShutdownCallback(Handler callback) {
    shutdownCallback = callback;
  }

  public Connection getConnection(long eq) {
    return conMap.get(eq);
  }

  public void initBufferPool(int initBufferNum, int bufferSize, int nextBufferNum) {
    this.bufferPool = new MemPool(this, initBufferNum, bufferSize, nextBufferNum);
  }

  public void reallocBufferPool() {
    this.bufferPool.realloc();
  }

  public void pushSendBuffer(long eq, int bufferId) {
    Connection connection = conMap.get(eq);
    assert(connection != null);
    connection.pushSendBuffer(bufferPool.getBuffer(bufferId));
  }

  public HpnlBuffer getSendBuffer(int bufferId) {
    return bufferPool.getBuffer(bufferId); 
  }

  public HpnlBuffer getRecvBuffer(int bufferId) {
    return bufferPool.getBuffer(bufferId);
  }

  public HpnlBuffer regRmaBuffer(ByteBuffer byteBuffer, int bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    rmaBufferMap.put(bufferId, byteBuffer);
    long rkey = reg_rma_buffer(byteBuffer, bufferSize, bufferId, nativeHandle);
    if (rkey < 0) {
      return null;
    }
    HpnlBuffer buffer = new HpnlBuffer(bufferId, byteBuffer, rkey);
    return buffer;
  }

  public HpnlBuffer regRmaBufferByAddress(ByteBuffer byteBuffer, long address, long bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    if (byteBuffer != null) {
      rmaBufferMap.put(bufferId, byteBuffer);
    }
    long rkey = reg_rma_buffer_by_address(address, bufferSize, bufferId, nativeHandle);
    if (rkey < 0) {
      return null;
    }
    HpnlBuffer buffer = new HpnlBuffer(bufferId, byteBuffer, rkey);
    return buffer;
  }

  public void unregRmaBuffer(int bufferId) {
    unreg_rma_buffer(bufferId, nativeHandle);
  }

  public HpnlBuffer getRmaBuffer(int bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    // allocate memory from on-heap, off-heap or AEP.
    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(bufferSize);
    long address = get_buffer_address(byteBuffer, nativeHandle);
    if (address < 0) {
      return null;  
    }
    rmaBufferMap.put(bufferId, byteBuffer);
    long rkey = reg_rma_buffer(byteBuffer, bufferSize, bufferId, nativeHandle);
    if (rkey < 0) {
      return null;
    }
    HpnlBuffer buffer = new HpnlBuffer(bufferId, byteBuffer, rkey, address);
    return buffer; 
  }

  public ByteBuffer getRmaBufferByBufferId(int rmaBufferId) {
    return rmaBufferMap.get(rmaBufferId); 
  }

  public void addReapConnection(Connection con) {
    reapCons.offer(con);
  }

  public boolean needReap() {
    return reapCons.size() > 0; 
  }

  public void pendingReap() {
    while (needReap()) {
      Connection con = reapCons.poll();
      con.shutdown();
    }
  }

  public int getWorkerNum() {
    return this.worker_num;
  }

  public long getNativeHandle() {
    return nativeHandle; 
  }

  public void free() {
    free(this.nativeHandle);
  }

  @Override
  public void setBuffer(ByteBuffer byteBuffer, int bufferSize, int bufferId) {
    set_buffer1(byteBuffer, bufferSize, bufferId, this.nativeHandle);
  }

  public int wait_eq_event() {
    return wait_eq_event1(this.nativeHandle); 
  }


  public void shutdown(long eq) {
    shutdown1(eq, this.nativeHandle);
  }

  public int delete_eq_event(long eq) {
    return delete_eq_event1(eq, this.nativeHandle);
  }

  private native void shutdown1(long eq, long nativeHandle);
  private native long native_connect(String ip, String port, long nativeHandle);
  private native int wait_eq_event1(long nativeHandle);
  private native int add_eq_event(long eq, long nativeHandle);
  private native int delete_eq_event1(long eq, long nativeHandle);
  private native void set_buffer1(ByteBuffer buffer, long size, int bufferId, long nativeHandle);
  private native long reg_rma_buffer(ByteBuffer buffer, long size, int bufferId, long nativeHandle);
  private native long reg_rma_buffer_by_address(long address, long size, int bufferId, long nativeHandle);
  private native void unreg_rma_buffer(int bufferId, long nativeHandle);
  private native long get_buffer_address(ByteBuffer buffer, long nativeHandle);
  private native int init(int worker_num_, int buffer_num_, boolean is_server_);
  private native void free(long nativeHandle);
  public native void finalize();

  public String getIp() {
    return ip;
  }

  public String getPort() {
    return port;
  }

  public void setCqService(CqService cqService) {
    this.cqService = cqService; 
  }

  private CqService cqService;

  private long nativeHandle;
  private long localEq;
  private String ip;
  private String port;
  private int worker_num;
  private int buffer_num;
  public boolean is_server;
  private Connection curCon;
  private ConcurrentHashMap<Long, CountDownLatch> connectLatchMap;
  private HashMap<Long, Connection> conMap;
  private LinkedBlockingQueue<Connection> reapCons;

  private ConcurrentHashMap<Integer, ByteBuffer> rmaBufferMap;

  private MemPool bufferPool;

  AtomicInteger rmaBufferId;

  private Handler connectedCallback;
  private Handler recvCallback;
  private Handler sendCallback;
  private Handler readCallback;
  private Handler shutdownCallback;

  private EqThread eqThread;
  private final AtomicBoolean needReap = new AtomicBoolean(false);
  private boolean needStop = false;


}
