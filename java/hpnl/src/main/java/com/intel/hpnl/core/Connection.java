package com.intel.hpnl.core;

import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingQueue;

public class Connection {

  public Connection(long nativeCon, EqService service) {
    this.service = service;
    this.sendBufferList = new LinkedBlockingQueue<Buffer>(16);
    init(nativeCon);
  }

  public void shutdown(long eq) {
    this.service.deregCon(eq);
    this.service.shutdown(eq);
    if (shutdownCallback != null) {
      shutdownCallback.handle(null, 0, 0);
    }
  }

  public native void recv(ByteBuffer buffer, int id);
  public native void send(int blockBufferSize, int rdmaBufferId);
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

  public void putSendBuffer(Buffer buffer) {
    try {
      sendBufferList.put(buffer);
    } catch (InterruptedException e) {
      e.printStackTrace(); 
    }
  }

  public Buffer getSendBuffer() {
    try {
      return sendBufferList.take();
    } catch (InterruptedException e) {
      e.printStackTrace(); 
      return null;
    }
  }

  public Buffer getRecvBuffer(int rdmaBufferId) {
    return service.getRecvBuffer(rdmaBufferId);
  }

  public void handleCallback(int eventType, int rdmaBufferId, int blockBufferSize) {
    if (eventType == EventType.CONNECTED_EVENT && connectedCallback != null) {
      connectedCallback.handle(this, rdmaBufferId, 0);
    } else if (eventType == EventType.RECV_EVENT && recvCallback != null) {
      recvCallback.handle(this, rdmaBufferId, blockBufferSize);
    } else if (eventType == EventType.SEND_EVENT) {
      putSendBuffer(service.getSendBuffer(rdmaBufferId));
      if (sendCallback != null) {
        sendCallback.handle(this, rdmaBufferId, blockBufferSize);
      }
    }   
  }

  EqService service;
 
  LinkedBlockingQueue<Buffer> sendBufferList;

  private Handler connectedCallback = null;
  private Handler recvCallback = null;
  private Handler sendCallback = null;
  private Handler shutdownCallback = null;

  private long nativeHandle;
}
