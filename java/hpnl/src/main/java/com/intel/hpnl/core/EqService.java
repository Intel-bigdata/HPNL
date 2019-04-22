package com.intel.hpnl.core;

import java.util.HashMap;
import java.nio.ByteBuffer;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;

public class EqService {
  private long nativeHandle;
  private long localEq;
  private int worker_num;
  private int buffer_num;
  public boolean is_server;
  protected Map<Long, Connection> conMap;
  private LinkedBlockingQueue<Connection> reapCons;

  private ConcurrentHashMap<Integer, ByteBuffer> rmaBufferMap;

  private MemPool sendBufferPool;
  private MemPool recvBufferPool;

  AtomicInteger rmaBufferId;

  private Handler connectedCallback;
  private Handler recvCallback;
  private Handler sendCallback;
  private Handler readCallback;
  private Handler shutdownCallback;

  private volatile CountDownLatch connectLatch;

  private EventTask eqTask;

  private Map<Long, Handler> connectedHandlers = new ConcurrentHashMap<>();

  static {
    System.loadLibrary("hpnl");
  }

  public EqService(int worker_num, int buffer_num, boolean is_server) {
    this.worker_num = worker_num;
    this.buffer_num = buffer_num;
    this.is_server = is_server;
    this.rmaBufferId = new AtomicInteger(0);

    this.conMap = new ConcurrentHashMap();
    this.reapCons = new LinkedBlockingQueue();
    this.rmaBufferMap = new ConcurrentHashMap();
    this.eqTask = new EqTask();
  }

  public EqService init() {
    if (init(worker_num, buffer_num, is_server) == -1)
      return null;
    return this; 
  }

  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    if(connectedCallback == null){
      throw new RuntimeException("connected callback cannot be null");
    }
    long id = sequenceId();
    localEq = internal_connect(ip, port, cqIndex, id, nativeHandle);
    if (localEq == -1) {
      return -1;
    }
    add_eq_event(localEq, nativeHandle);
    Handler prv = connectedHandlers.putIfAbsent(id, connectedCallback);
    if(prv != null){
      throw new RuntimeException("non-unique id found, "+id);
    }
    return 0;
  }

  public int connect(String ip, String port, int cqIndex, long timeoutMill) {
    if (!is_server) {
      synchronized (connectLatch) {
        if(connectLatch != null){
          throw new RuntimeException("the last connection is still under going");
        }
        connectLatch = new CountDownLatch(1);
      }
    }
    int rt = connect(ip, port, cqIndex, null);
    if(rt < 0){
      if (!is_server) {
        connectLatch.countDown();
      }
      return rt;
    }
    if (!is_server) {
      waitToConnected(timeoutMill);
    }
    return 0;
  }

  private void waitToConnected(long timeoutMill) {
    try {
      this.connectLatch.await(timeoutMill, TimeUnit.MILLISECONDS);
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  private void waitToComplete() {
    try {
      eqTask.waitToComplete();
    } catch (InterruptedException e) {
      e.printStackTrace();
    } finally {
    }
  }

  public void stop() {
    for (Connection con : reapCons) {
      addReapCon(con);
    }
    synchronized(this) {
      eqTask.stop();
    }
    delete_eq_event(localEq, nativeHandle);
    waitToComplete();
  }

  private void regCon(long eq, long con,
                      String dest_addr, int dest_port, String src_addr, int src_port, long connectId) {
    Connection connection = new Connection(eq, con, this, connectId);
    connection.setAddrInfo(dest_addr, dest_port, src_addr, src_port);
    conMap.put(eq, connection);
  }

  public void unregCon(long eq) {
    if (conMap.containsKey(eq)) {
      conMap.remove(eq);
    }
    if (!is_server) {
      eqTask.stop();
    }
  }

  private static long sequenceId() {
    return Math.abs(UUID.randomUUID().getLeastSignificantBits());
  }

  protected void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = conMap.get(eq);
    if (eventType == EventType.CONNECTED_EVENT) {
      long id = connection.getConnectId();
      Handler connectedHandler = connectedHandlers.remove(id);
      if(connectedHandler != null){
        connectedHandler.handle(connection, 0, 0);
      }
    }
    if (this.connectLatch != null && eventType == EventType.CONNECTED_EVENT) {
      synchronized (connectLatch) {
        if(connectLatch != null) {
          this.connectLatch.countDown();
          this.connectLatch = null;
        }
      }
    }
  }

  public void setConnectedCallback(Handler callback) {
    throw new UnsupportedOperationException();
  }

//  public void setRecvCallback(Handler callback) {
//    recvCallback = callback;
//  }
//
//  public void setSendCallback(Handler callback) {
//    sendCallback = callback;
//  }
//
//  public void setReadCallback(Handler callback) {
//    readCallback = callback;
//  }

//  public void setShutdownCallback(Handler callback) {
//    shutdownCallback = callback;
//  }

  public Connection getCon(long eq) {
    return conMap.get(eq);
  }

  public long getNativeHandle() {
    return nativeHandle;
  }

  public void initBufferPool(int initBufferNum, int bufferSize, int nextBufferNum) {
    this.sendBufferPool = new MemPool(this, initBufferNum, bufferSize, nextBufferNum, MemPool.Type.SEND);
    this.recvBufferPool = new MemPool(this, initBufferNum*2, bufferSize, nextBufferNum*2, MemPool.Type.RECV);
  }

  public void reallocBufferPool() {
    this.sendBufferPool.realloc();
    this.recvBufferPool.realloc();
  }

  public void putSendBuffer(long eq, int rdmaBufferId) {
    Connection connection = conMap.get(eq);
    connection.putSendBuffer(sendBufferPool.getBuffer(rdmaBufferId));
  }

  public RdmaBuffer getSendBuffer(int rdmaBufferId) {
    return sendBufferPool.getBuffer(rdmaBufferId); 
  }

  public RdmaBuffer getRecvBuffer(int rdmaBufferId) {
    return recvBufferPool.getBuffer(rdmaBufferId);
  }

  public RdmaBuffer regRmaBuffer(ByteBuffer byteBuffer, int bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    rmaBufferMap.put(bufferId, byteBuffer);
    long rkey = reg_rma_buffer(byteBuffer, bufferSize, bufferId, nativeHandle);
    if (rkey < 0) {
      return null;
    }
    RdmaBuffer buffer = new RdmaBuffer(bufferId, byteBuffer, rkey);
    return buffer;
  }

  public RdmaBuffer regRmaBufferByAddress(ByteBuffer byteBuffer, long address, long bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    if (byteBuffer != null) {
      rmaBufferMap.put(bufferId, byteBuffer);
    }
    long rkey = reg_rma_buffer_by_address(address, bufferSize, bufferId, nativeHandle);
    if (rkey < 0) {
      return null;
    }
    RdmaBuffer buffer = new RdmaBuffer(bufferId, byteBuffer, rkey);
    return buffer;
  }

  public void unregRmaBuffer(int rdmaBufferId) {
    unreg_rma_buffer(rdmaBufferId, nativeHandle);
  }

  public RdmaBuffer getRmaBuffer(int bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    // allocate memory from on-heap, off-heap or AEP.
    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(bufferSize);
    long address = get_buffer_address(byteBuffer, nativeHandle);
    rmaBufferMap.put(bufferId, byteBuffer);
    long rkey = reg_rma_buffer(byteBuffer, bufferSize, bufferId, nativeHandle);
    if (rkey < 0) {
      return null;
    }
    RdmaBuffer buffer = new RdmaBuffer(bufferId, byteBuffer, rkey, address);
    return buffer; 
  }

  public ByteBuffer getRmaBufferByBufferId(int rmaBufferId) {
    return rmaBufferMap.get(rmaBufferId); 
  }

  public void addReapCon(Connection con) {
    reapCons.offer(con);
  }

  public boolean needReap() {
    return reapCons.size() > 0; 
  }

  public void externalEvent() {
    while (needReap()) {
      Connection con = reapCons.poll();
      con.shutdown();
    }
  }

  public int getWorkerNum() {
    return this.worker_num;
  }

  public EventTask getEventTask(){
    return eqTask;
  }

  public native void shutdown(long eq, long nativeHandle);
  private native long internal_connect(String ip, String port, int cqIndex, long connectId, long nativeHandle);
  public native int wait_eq_event(long nativeHandle);
  public native int add_eq_event(long eq, long nativeHandle);
  public native int delete_eq_event(long eq, long nativeHandle);
  public native void set_recv_buffer(ByteBuffer buffer, long size, int rdmaBufferId, long nativeHandle);
  public native void set_send_buffer(ByteBuffer buffer, long size, int rdmaBufferId, long nativeHandle);
  private native long reg_rma_buffer(ByteBuffer buffer, long size, int rdmaBufferId, long nativeHandle);
  private native long reg_rma_buffer_by_address(long address, long size, int rdmaBufferId, long nativeHandle);
  private native void unreg_rma_buffer(int rdmaBufferId, long nativeHandle);
  private native long get_buffer_address(ByteBuffer buffer, long nativeHandle);
  private native int init(int worker_num_, int buffer_num_, boolean is_server_);
  private native void free(long nativeHandle);
  public native void finalize();

  protected class EqTask extends EventTask {

    @Override
    public void loopEvent() {
      while (running.get() || needReap()) {
        if (wait_eq_event(getNativeHandle()) == -1) {
          stop();
        }
        externalEvent();
      }
    }
  }
}
