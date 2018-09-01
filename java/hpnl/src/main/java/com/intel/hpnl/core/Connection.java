package com.intel.hpnl.core;

import java.nio.ByteBuffer;

public class Connection {

  public Connection(long nativeCon) {
    init(nativeCon);
  }

  public native void recv(ByteBuffer buffer, int id);
  public native void send(ByteBuffer buffer, int rdmaBufferId, int blockBufferSize, int blockBufferId, long seq);
  public native void shutdown();
  private native void init(long eq);
  public native void finalize();

  public Handler getConnectedCallback() {
    return connectedCallback;
  }

  public void setConnectedCallback(Handler callback) {
    connectedCallback = callback; 
  }

  public Handler getRecvCallback() {
    return recvCallback; 
  }

  public void setRecvCallback(Handler callback) {
    recvCallback = callback; 
  }

  public Handler getSendCallback() {
    return sendCallback; 
  }

  public void setSendCallback(Handler callback) {
    sendCallback = callback; 
  }

  public Handler getShutdownCallback() {
    return shutdownCallback; 
  }

  public void setShutdownCallback(Handler callback) {
    shutdownCallback = callback; 
  }

  public void handleCallback(int eventType, int rdmaBufferId, int blockBufferSize, int blockBufferId, long seq) {
    if (eventType == EventType.CONNECTED_EVENT && connectedCallback != null) {
      connectedCallback.handle(this, rdmaBufferId, 0, 0, seq);
    } else if (eventType == EventType.RECV_EVENT && recvCallback != null) {
      recvCallback.handle(this, rdmaBufferId, blockBufferSize, blockBufferId, seq);
    } else if (eventType == EventType.SEND_EVENT && sendCallback != null) {
      sendCallback.handle(this, rdmaBufferId, blockBufferSize, blockBufferId, seq);
    } else if (eventType == EventType.SHUTDOWN && shutdownCallback != null) {
      shutdownCallback.handle(this, rdmaBufferId, 0, 0, seq);
    }
  }

  private Handler connectedCallback;
  private Handler recvCallback;
  private Handler sendCallback;
  private Handler shutdownCallback;

  private long nativeHandle;
}
