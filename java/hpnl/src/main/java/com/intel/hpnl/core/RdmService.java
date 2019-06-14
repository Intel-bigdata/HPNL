package com.intel.hpnl.core;

import java.nio.ByteBuffer;
import java.util.concurrent.ConcurrentHashMap;

public class RdmService {
  static {
    System.loadLibrary("hpnl");
  }

  public RdmService(int buffer_num, boolean is_server) {
    this.buffer_num = buffer_num;
    this.is_server = is_server;

    conMap = new ConcurrentHashMap<Long, RdmConnection>();
  }

  public RdmService init() {
    init(buffer_num, is_server);
    this.worker = new RdmThread(this);
    this.worker.start();
    return this;
  }

  public void listen(String ip, String port) {
    listen(ip, port, nativeHandle);
  }

  public void join() {
    try {
      this.worker.join(); 
    } catch (InterruptedException e) {
      e.printStackTrace();
    } finally {
    }
  }

  public void shutdown() {
    this.worker.shutdown();
  }

  public RdmConnection get_con(String ip, String port) {
    return conMap.get(get_con(ip, port, nativeHandle));
  }

  public int wait_event() {
    return wait_event(this.nativeHandle);
  }

  private void handleCallback(long handle, int eventType, int blockId, int blockSize) {
    RdmConnection connection = conMap.get(handle);
    if (connection == null) {
      throw new NullPointerException("connection is NULL when handling " + eventType + " event.");
    }
    connection.handleCallback(eventType, blockId, blockSize);
  }

  public void free() {
    free(this.nativeHandle);
  }

  private void regCon(long con_handle) {
    RdmConnection con = new RdmConnection(con_handle, this);
    con.setRecvCallback(recvCallback);
    con.setSendCallback(sendCallback);
    conMap.put(con_handle, con);
  }

  public void initRecvBufferPool(int initBufferNum, int bufferSize, int nextBufferNum) {
    this.recvBufferPool = new MemPool(this, initBufferNum, bufferSize, nextBufferNum, MemPool.Type.RECV);
  }

  public void initSendBufferPool(int initBufferNum, int bufferSize, int nextBufferNum) {
    this.sendBufferPool = new MemPool(this, initBufferNum, bufferSize, nextBufferNum, MemPool.Type.SEND);
  }

  public void reallocBufferPool() {
    this.sendBufferPool.realloc();
    this.recvBufferPool.realloc();
  }

  public void setRecvCallback(RdmHandler callback) {
    recvCallback = callback;
  }

  public void setSendCallback(RdmHandler callback) {
    sendCallback = callback;
  }

  public void pushSendBuffer(long handle, int bufferId) {
    RdmConnection connection = conMap.get(handle);
    if (connection == null) {
      throw new NullPointerException("connection is null when putting " + bufferId + " bufferId"); 
    }
    connection.pushSendBuffer(sendBufferPool.getBuffer(bufferId));
  }

  public HpnlBuffer getSendBuffer(int bufferId) {
    return sendBufferPool.getBuffer(bufferId); 
  }

  public HpnlBuffer getRecvBuffer(int bufferId) {
    return recvBufferPool.getBuffer(bufferId);
  }

  public long getNativeHandle() {
    return nativeHandle;
  }

  private native int init(int buffer_num, boolean is_server);
  private native void listen(String ip, String port, long nativeHandle);
  private native long get_con(String ip, String port, long nativeHandle);
  private native int wait_event(long nativeHandle);
  public native void set_recv_buffer(ByteBuffer buffer, long size, int bufferId, long nativeHandle);
  public native void set_send_buffer(ByteBuffer buffer, long size, int bufferId, long nativeHandle);
  private native void free(long nativeHandle);

  private String addr;
  private int port;
  private int buffer_num;
  private boolean is_server;
  private RdmThread worker;
  private ConcurrentHashMap<Long, RdmConnection> conMap;
  private RdmHandler recvCallback;
  private RdmHandler sendCallback;
  private MemPool sendBufferPool;
  private MemPool recvBufferPool;

  private long nativeHandle;
}
