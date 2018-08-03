package com.intel.hpnl.core;

import java.nio.ByteBuffer;

public class Connection {

  public Connection(long nativeCon) {
    init(nativeCon);
  }

  public native void read(ByteBuffer buffer, int id);
  public native void write(String str, int id, int return_id);
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

  public void handleCallback(int eventType, int blockId) {
    if (eventType == EventType.CONNECTED_EVENT && connectedCallback != null) {
      connectedCallback.handle(this, blockId);
    } else if (eventType == EventType.RECV_EVENT && recvCallback != null) {
      recvCallback.handle(this, blockId); 
    } else if (eventType == EventType.SEND_EVENT && sendCallback != null) {
      sendCallback.handle(this, blockId); 
    } else if (eventType == EventType.SHUTDOWN && shutdownCallback != null) {
      shutdownCallback.handle(this, blockId); 
    }
  }

  private Handler connectedCallback;
  private Handler recvCallback;
  private Handler sendCallback;
  private Handler shutdownCallback;

  private long nativeHandle;
}
