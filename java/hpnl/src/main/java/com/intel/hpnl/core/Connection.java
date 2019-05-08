package com.intel.hpnl.core;

import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingQueue;

public class Connection {

  private EqService service;
  private CqService cqService;

  private LinkedBlockingQueue<RdmaBuffer> sendBufferList;

  private String destAddr;
  private int destPort;
  private String srcAddr;
  private int srcPort;

  private boolean connected;

  private Handler connectedCallback = null;
  private Handler recvCallback = null;
  private Handler sendCallback = null;
  private Handler readCallback = null;
  private Handler shutdownCallback = null;
  private Handler generalCallback;

  private long nativeHandle;
  private final long nativeEq;

  private int cqIndex;
  private final long connectId;

  public Connection(long nativeEq, long nativeCon, EqService service, CqService cqService, long connectId) {
    this.service = service;
    this.cqService = cqService;
    this.sendBufferList = new LinkedBlockingQueue<>();
    this.nativeEq = nativeEq;
    init(nativeCon);
    cqIndex = get_cq_index(this.nativeHandle);
    connected = true;
    this.connectId = connectId;
  }

  public void shutdown(){
    if(!connected){
      return;
    }
    synchronized (this) {
      if(!connected){
        return;
      }
      this.service.unregCon(nativeEq);
      this.service.shutdown(nativeEq, service.getNativeHandle());
      this.service.delete_eq_event(nativeEq, service.getNativeHandle());
      deleteGlobalRef(this.nativeHandle);
      if (shutdownCallback != null) {
        shutdownCallback.handle(null, 0, 0);
      }
      free(nativeHandle);
      connected = false;
    }
  }

  public void recv(ByteBuffer buffer, int id) {
    recv(buffer, id, this.nativeHandle);
  }

  public int send(int blockBufferSize, int rdmaBufferId) {
    return send(blockBufferSize, rdmaBufferId, this.nativeHandle);
  }

  public int read(int rdmaBufferId, int localOffset, long len, long remoteAddr, long remoteMr) {
    return read(rdmaBufferId, localOffset, len, remoteAddr, remoteMr, this.nativeHandle);
  }

  public void releaseRecvBuffer(int rdmaBufferId){
    releaseRecvBuffer(rdmaBufferId, nativeHandle);
  }

  private native void recv(ByteBuffer buffer, int id, long nativeHandle);
  private native int send(int blockBufferSize, int rdmaBufferId, long nativeHandle);
  private native int read(int rdmaBufferId, int localOffset, long len, long remoteAddr, long remoteMr, long nativeHandle);

  private native void init(long eq);
  private native int get_cq_index(long nativeHandle);
  public native void finalize();
  private native void releaseRecvBuffer(int rdmaBufferId, long nativeHandle);
  private native void deleteGlobalRef(long nativeHandle);
  private native void free(long nativeHandle);

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

  public void setReadCallback(Handler callback) {
    readCallback = callback; 
  }

  public Handler getShutdownCallback() {
    return shutdownCallback; 
  }

  public void setShutdownCallback(Handler callback) {
    shutdownCallback = callback; 
  }

  public void setGeneralEventCallback(Handler callback){
    this.generalCallback = callback;
  }

  public Handler getGeneralCallback() {
    return generalCallback;
  }

  public void putSendBuffer(RdmaBuffer buffer) {
    try {
      sendBufferList.put(buffer);
    } catch (InterruptedException e) {
      e.printStackTrace(); 
    }
  }

  public RdmaBuffer takeSendBuffer(boolean wait) {
    if (wait) {
      try {
        return sendBufferList.take();
      } catch (InterruptedException e) {
        e.printStackTrace(); 
      }
      return null;
    } else {
      return sendBufferList.poll();
    }
  }

  public RdmaBuffer getSendBuffer(int rdmaBufferId){
    return service.getSendBuffer(rdmaBufferId);
  }

  public RdmaBuffer getRecvBuffer(int rdmaBufferId) {
    return service.getRecvBuffer(rdmaBufferId);
  }

  public ByteBuffer getRmaBuffer(int rmaBufferId) {
    return service.getRmaBufferByBufferId(rmaBufferId);
  }

  public void setAddrInfo(String destAddr, int destPort, String srcAddr, int srcPort) {
    this.destAddr = destAddr;
    this.destPort = destPort;
    this.srcAddr = srcAddr;
    this.srcPort = srcPort;
  }

  public String getDestAddr() {
    return this.destAddr;
  }

  public int getDestPort() {
    return this.destPort;
  }

  public String getSrcAddr() {
    return this.srcAddr; 
  }

  public int getSrcPort() {
    return this.srcPort; 
  }

  public int getCqIndex(){
    return cqIndex;
  }

  public long getConnectId() {
    return connectId;
  }

  public int handleCallback(int eventType, int rdmaBufferId, int blockBufferSize) {
    int e;
    switch (eventType){
      case EventType.RECV_EVENT:
        e = executeCallback(recvCallback, rdmaBufferId, blockBufferSize);
        break;
      case EventType.SEND_EVENT:
        e = executeCallback(sendCallback, rdmaBufferId, blockBufferSize);
        putSendBuffer(service.getSendBuffer(rdmaBufferId));
        break;
      case EventType.READ_EVENT:
        e = executeCallback(readCallback, rdmaBufferId, blockBufferSize);
        break;
      case EventType.CONNECTED_EVENT:
        e = executeCallback(connectedCallback, rdmaBufferId, 0);
        break;
      default:
        e = Handler.RESULT_DEFAULT;
    }
    //general event callback
    executeCallback(getGeneralCallback(), -1, -1);
    return e;
  }

  private int executeCallback(Handler handler, int rdmaBufferId, int blockBufferSize){
    if(handler == null){
      return Handler.RESULT_DEFAULT;
    }
    try{
      return handler.handle(this, rdmaBufferId, blockBufferSize);
    }catch(Exception e){
      e.printStackTrace();
      return -1;
    }
  }
}
