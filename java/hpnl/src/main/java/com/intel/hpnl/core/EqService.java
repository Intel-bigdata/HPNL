package com.intel.hpnl.core;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.nio.ByteBuffer;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;

public class EqService {
  static {
    System.load("/usr/local/lib/libhpnl.so");
  }

  public EqService(String ip, String port, int buffer_num, boolean is_server) {
    this.ip = ip;
    this.port = port;
    this.is_server = is_server;
    this.rmaBufferId = new AtomicInteger(0);

    this.conMap = new HashMap<Long, Connection>();
    this.reapCons = new ConcurrentHashMap<Long, Connection>();

    this.sendBufferMap = new HashMap<Integer, RdmaBuffer>();
    this.recvBufferMap = new HashMap<Integer, RdmaBuffer>();
    this.rmaBufferMap = new ConcurrentHashMap<Integer, ByteBuffer>();

    init(ip, port, buffer_num, is_server);

    eqThread = new EqThread(this);
  }

  public void start(int con_num) {
    if (is_server) {
      con_num = 1;
    } else {
      connectLatch = new CountDownLatch(con_num);
    }
    for (int i = 0; i < con_num; i++) {
      localEq = connect();
      add_eq_event(localEq);
    }
    eqThread.start();
  }

  public void waitToConnected() {
    try {
      this.connectLatch.await();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  public void join() {
    try {
      eqThread.join();
    } catch (InterruptedException e) {
      e.printStackTrace();
    } finally {
      synchronized(this) {
        free();
      }
    }
  }

  public void shutdown() {
    for (Map.Entry<Long, Connection> entry: conMap.entrySet()) {
      addReapCon(entry.getKey(), entry.getValue());
    }
    synchronized(this) {
      eqThread.shutdown();
    }
    delete_eq_event(localEq);
  }

  private void registerCon(long eq, long con) {
    Connection connection = new Connection(con, this);
    conMap.put(eq, connection);
  }

  public void deregCon(long eq) {
    if (conMap.containsKey(eq)) {
      conMap.remove(eq);
    }
    if (!is_server) {
      eqThread.shutdown(); 
    }
  }

  private void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = conMap.get(eq);
    if (eventType == EventType.CONNECTED_EVENT) {
      connection.setConnectedCallback(connectedCallback);
      connection.setRecvCallback(recvCallback);
      connection.setSendCallback(sendCallback);
      connection.setReadCallback(readCallback);
      connection.setShutdownCallback(shutdownCallback);
    }
    connection.handleCallback(eventType, 0, 0);
    if (!is_server && eventType == EventType.CONNECTED_EVENT) {
      this.connectLatch.countDown();
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

  public HashMap<Long, Connection> getConMap() {
    return conMap;
  }

  public long getNativeHandle() {
    return nativeHandle;
  }

  public void setRecvBuffer(ByteBuffer byteBuffer, long size, int rdmaBufferId) {
    RdmaBuffer buffer = new RdmaBuffer(rdmaBufferId, byteBuffer);
    recvBufferMap.put(rdmaBufferId, buffer);
    set_recv_buffer(byteBuffer, size, rdmaBufferId);
  }

  public void setSendBuffer(ByteBuffer byteBuffer, long size, int rdmaBufferId) {
    RdmaBuffer buffer = new RdmaBuffer(rdmaBufferId, byteBuffer);
    sendBufferMap.put(rdmaBufferId, buffer);
    set_send_buffer(byteBuffer, size, rdmaBufferId);
  }

  public void putSendBuffer(long eq, int rdmaBufferId) {
    Connection connection = conMap.get(eq);
    connection.putSendBuffer(sendBufferMap.get(rdmaBufferId));
  }

  public RdmaBuffer getRecvBuffer(int rdmaBufferId) {
    return recvBufferMap.get(rdmaBufferId);
  }

  public RdmaBuffer getSendBuffer(int rdmaBufferId) {
    return sendBufferMap.get(rdmaBufferId); 
  }

  public RdmaBuffer regRmaBuffer(ByteBuffer byteBuffer, int bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    rmaBufferMap.put(bufferId, byteBuffer);
    long rkey = reg_rma_buffer(byteBuffer, bufferSize, bufferId);
    RdmaBuffer buffer = new RdmaBuffer(bufferId, byteBuffer, rkey);
    return buffer;
  }

  public RdmaBuffer regRmaBufferByAddress(ByteBuffer byteBuffer, long address, int bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    if (byteBuffer != null) {
      rmaBufferMap.put(bufferId, byteBuffer);
    }
    long rkey = reg_rma_buffer_by_address(address, bufferSize, bufferId);
    RdmaBuffer buffer = new RdmaBuffer(bufferId, byteBuffer, rkey);
    return buffer;
  }

  public void unregRmaBuffer(int rdmaBufferId) {
    unreg_rma_buffer(rdmaBufferId);
  }

  public RdmaBuffer getRmaBuffer(int bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    // allocate memory from on-heap, off-heap or AEP.
    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(bufferSize);
    long address = get_buffer_address(byteBuffer);
    rmaBufferMap.put(bufferId, byteBuffer);
    long rkey = reg_rma_buffer(byteBuffer, bufferSize, bufferId);
    RdmaBuffer buffer = new RdmaBuffer(bufferId, byteBuffer, rkey, address);
    return buffer; 
  }

  public ByteBuffer getRmaBufferByBufferId(int rmaBufferId) {
    return rmaBufferMap.get(rmaBufferId); 
  }

  public void addReapCon(Long eq, Connection con) {
    reapCons.put(eq, con);
  }

  public Map<Long, Connection> getReapCon() {
    return reapCons; 
  }

  public void externalEvent() {
    for (Iterator<Map.Entry<Long, Connection>> it = reapCons.entrySet().iterator(); it.hasNext();){
      Map.Entry<Long, Connection> item = it.next();
      item.getValue().shutdown(item.getKey());
      it.remove();
    }
  }

  public native void shutdown(long eq);
  private native long connect();
  public native int wait_eq_event();
  public native int add_eq_event(long eq);
  public native int delete_eq_event(long eq);
  public native void set_recv_buffer(ByteBuffer buffer, long size, int rdmaBufferId);
  public native void set_send_buffer(ByteBuffer buffer, long size, int rdmaBufferId);
  private native long reg_rma_buffer(ByteBuffer buffer, long size, int rdmaBufferId);
  private native long reg_rma_buffer_by_address(long address, long size, int rdmaBufferId);
  private native void unreg_rma_buffer(int rdmaBufferId);
  private native long get_buffer_address(ByteBuffer buffer);
  private native void init(String ip_, String port_, int buffer_num_, boolean is_server_);
  private native void free();
  public native void finalize();

  private long nativeHandle;
  private long localEq;
  private String ip;
  private String port;
  public boolean is_server;
  private HashMap<Long, Connection> conMap;
  private ConcurrentHashMap<Long, Connection> reapCons;

  private HashMap<Integer, RdmaBuffer> sendBufferMap;
  private HashMap<Integer, RdmaBuffer> recvBufferMap;
  private ConcurrentHashMap<Integer, ByteBuffer> rmaBufferMap;

  AtomicInteger rmaBufferId;

  private Handler connectedCallback;
  private Handler recvCallback;
  private Handler sendCallback;
  private Handler readCallback;
  private Handler shutdownCallback;

  private EqThread eqThread;
  private final AtomicBoolean needReap = new AtomicBoolean(false);
  private boolean needStop = false;
  private CountDownLatch connectLatch;
}
