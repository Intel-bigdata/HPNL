package com.intel.hpnl.core;

import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingQueue;

public class RdmConnection {
  public RdmConnection(long nativeHandle, RdmService rdmService) {
    this.nativeHandle = nativeHandle;
    this.rdmService = rdmService;
    this.sendBufferList = new LinkedBlockingQueue<HpnlBuffer>();
    this.localNameLength = get_local_name_length(this.nativeHandle);
    this.localName = ByteBuffer.allocateDirect(localNameLength);
    get_local_name(this.localName, this.nativeHandle);
    this.localName.limit(localNameLength);
    init(this.nativeHandle);
  }

  public RdmHandler getRecvCallback() {
    return recvCallback; 
  }

  public void setRecvCallback(RdmHandler callback) {
    this.recvCallback = callback; 
  }

  public RdmHandler getSendCallback() {
    return sendCallback; 
  }

  public void setSendCallback(RdmHandler callback) {
    sendCallback = callback; 
  }

  public void handleCallback(int eventType, int bufferId, int blockBufferSize) {
    Exception e = null;
    if (eventType == EventType.RECV_EVENT) {
      e = executeCallback(recvCallback, bufferId, blockBufferSize);
    } else if (eventType == EventType.SEND_EVENT) {
      e = executeCallback(sendCallback, bufferId, blockBufferSize);
      pushSendBuffer(rdmService.getSendBuffer(bufferId));
    } else {
    }
    if(e != null){
      e.printStackTrace();
    }
  }

  public void pushSendBuffer(HpnlBuffer buffer) {
    try {
      sendBufferList.put(buffer);
    } catch (InterruptedException e) {
      e.printStackTrace(); 
    }
  }

  private HpnlBuffer takeSendBuffer() {
    return sendBufferList.poll();
  }

  public HpnlBuffer getRecvBuffer(int bufferId) {
    return this.rdmService.getRecvBuffer(bufferId);
  }

  private Exception executeCallback(RdmHandler handler, int bufferId, int blockBufferSize){
    if (handler == null) {
      return null;
    }
    try{
      handler.handle(this, bufferId, blockBufferSize);
    }catch(Exception e){
      return e;
    }
    return null;
  }

  public void send(ByteBuffer buffer, byte b, long seq) {
    HpnlBuffer hpnlBuffer = takeSendBuffer();
    hpnlBuffer.put(buffer, localNameLength, localName, b, seq);
    send(hpnlBuffer.size(), hpnlBuffer.getBufferId(), this.nativeHandle);
  }

  public void sendTo(ByteBuffer buffer, byte b, long seq, ByteBuffer peerName) {
    HpnlBuffer hpnlBuffer = takeSendBuffer();
    hpnlBuffer.put(buffer, localNameLength, localName, b, seq);
    sendTo(hpnlBuffer.size(), hpnlBuffer.getBufferId(), peerName, this.nativeHandle);
  }

  private native void init(long nativeHandle);
  private native void get_local_name(ByteBuffer localName, long nativeHandle);
  private native int get_local_name_length(long nativeHandle);
  private native int send(int blockBufferSize, int bufferId, long nativeHandle);
  private native int sendTo(int blockBufferSize, int bufferId, ByteBuffer peerName, long nativeHandle);
  // 1 byte -> 0(connection) or 1(nonconnection)
  // 4 bytes -> address length
  // N bytes -> address
  // 1 byte -> type
  // 8 bytes -> seq id
  // N bytes -> raw message
  public native int sendBuf(ByteBuffer buffer, int bufferSize, long nativeHandle);
  public native int sendBufTo(ByteBuffer buffer, int bufferSize, ByteBuffer peerName, long nativeHandle);

  RdmService rdmService;
  ByteBuffer localName;
  int localNameLength;
  private LinkedBlockingQueue<HpnlBuffer> sendBufferList;
  private RdmHandler recvCallback = null;
  private RdmHandler sendCallback = null;

  private long nativeHandle;
}
