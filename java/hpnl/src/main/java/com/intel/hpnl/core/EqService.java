package com.intel.hpnl.core;

import java.util.HashMap;
import java.nio.ByteBuffer;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.CountDownLatch;

public class EqService {
  static {
    System.load("/usr/local/lib/libhpnl.so");
  }

  public EqService(String ip, String port, boolean is_server) {
    this.ip = ip;
    this.port = port;
    this.is_server = is_server;

    this.eqs = new ConcurrentHashMap<Long, Integer>();
    this.conMap = new HashMap<Long, Connection>();

    init(ip, port, is_server);

    eqThread = new EqThread(this);
  }

  public void start(int num) {
    if (is_server) {
      num = 1;
    } else {
      connectLatch = new CountDownLatch(num);
    }
    for (int i = 0; i < num; i++) {
      long eq = connect();
      registerEq(eq);
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
    }
  }

  public void shutdown() {
    needReap.set(true);
    synchronized(this) {
      if (!needStop) {
        needStop = true;
        this.notify();
      }
    }
  }

  public void waitToStop() {
    synchronized(this) {
      while (!needStop) {
        try {
          this.wait();
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
      }
    }
  }

  public boolean maybeStop() {
    if (conInflight == 0 && needReap.get() == true) {
      return true;
    } else {
      return false;
    }
  }

  public void incConNum() {
    conInflight++;
  }

  public void decConNum() {
    conInflight--;
  }

  private void registerEq(long eq) {
    eqs.put(eq, 1);
  }

  private void registerCon(long eq, long con) {
    Connection connection = new Connection(con);
    conMap.put(eq, connection);
  }

  private void deregCon(long eq) {
    eqs.put(eq, 0);
    conMap.remove(eq);
  }

  private void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = conMap.get(eq);
    if (eventType == EventType.CONNECTED_EVENT) {
      connection.setConnectedCallback(connectedCallback);
      connection.setRecvCallback(recvCallback);
      connection.setSendCallback(sendCallback);
      connection.setShutdownCallback(shutdownCallback);
    }
    connection.handleCallback(eventType, 0, 0, 0, 0);
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

  public void setShutdownCallback(Handler callback) {
    shutdownCallback = callback;
  }

  public ConcurrentHashMap<Long, Integer> getEqs() {
    return eqs;
  }

  public HashMap<Long, Connection> getConMap() {
    return conMap;
  }

  public long getNativeHandle() {
    return nativeHandle;
  }

  private native long connect();
  public native int wait_eq_event(long eq);
  public native void set_recv_buffer(ByteBuffer buffer, long size);
  public native void set_send_buffer(ByteBuffer buffer, long size);
  private native void init(String ip_, String port_, boolean is_server_);
  public native void finalize();

  private long nativeHandle;
  private String ip;
  private String port;
  public boolean is_server;
  private HashMap<Long, Connection> conMap;
  private ConcurrentHashMap<Long, Integer> eqs;

  private Handler connectedCallback;
  private Handler recvCallback;
  private Handler sendCallback;
  private Handler shutdownCallback;

  private EqThread eqThread;
  private int conInflight = 0;
  private final AtomicBoolean needReap = new AtomicBoolean(false);
  private boolean needStop = false;
  private CountDownLatch connectLatch;
}
