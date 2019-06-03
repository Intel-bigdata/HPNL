package com.intel.hpnl.core;

import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingQueue;

public class Connection {

  public Connection(long nativeEq, long nativeCon, int index, long threadId,  EqService eqService, CqService cqService) {
    this.eqService = eqService;
    this.cqService = cqService;
    this.sendBufferList = new LinkedBlockingQueue<HpnlBuffer>();
    this.nativeEq = nativeEq;
    this.index = index;
    this.threadId = threadId;
    init(nativeCon);
    connected = true;
  }

  public void shutdown(){
    if(!connected){
      return;
    }
    synchronized (this) {
      if(!connected){
        return;
      }
      this.eqService.shutdown(nativeEq, eqService.getNativeHandle());
      this.eqService.delete_eq_event(nativeEq, eqService.getNativeHandle());
      this.eqService.unregCon(nativeEq);
      if (shutdownCallback != null) {
        shutdownCallback.handle(this, 0, 0);
      }
      connected = false;
    }
  }

  public void recv(ByteBuffer buffer, int id) {
    recv(buffer, id, this.nativeHandle);
  }

  public int send(ByteBuffer buffer, byte b, long seq) {
    if (Thread.currentThread().getId() != this.threadId) {
      this.cqService.addExternalEvent(this.index, new ExternalHandler() {
        public void handle() {
          send(buffer, b, seq);
        } 
      });
      return 0;
    } else {
      HpnlBuffer hpnlBuffer = takeSendBuffer();
      if (hpnlBuffer == null) {
        this.cqService.addExternalEvent(this.index, new ExternalHandler() {
          public void handle() {
            send(buffer, b, seq);
          } 
        });
      } else {
        hpnlBuffer.put(buffer, b, seq);
        send(hpnlBuffer.size(), hpnlBuffer.getBufferId(), this.nativeHandle);
      }
    }
    return 0; 
  }

  public int read(int bufferId, int localOffset, long len, long remoteAddr, long remoteMr) {
    int res = read(bufferId, localOffset, len, remoteAddr, remoteMr, this.nativeHandle);
    if (res == -11) {
      this.cqService.addExternalEvent(this.index, new ExternalHandler() {
        public void handle() {
          read(bufferId, localOffset, len, remoteAddr, remoteMr);
        } 
      });
    }
    return 0;
  }

  public void delCon() {
    this.cqService.addExternalEvent(this.index, new ExternalHandler() {
      public void handle() {
        free(nativeHandle);
      }
    });
  }

  public native void recv(ByteBuffer buffer, int id, long nativeHandle);
  public native int send(int blockBufferSize, int bufferId, long nativeHandle);
  public native int read(int bufferId, int localOffset, long len, long remoteAddr, long remoteMr, long nativeHandle);
  private native void init(long eq);
  public native void finalize();
  public native void free(long nativeHandle);

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

  public void putSendBuffer(HpnlBuffer buffer) {
    try {
      sendBufferList.put(buffer);
    } catch (InterruptedException e) {
      e.printStackTrace(); 
    }
  }

  private HpnlBuffer takeSendBuffer() {
    return sendBufferList.poll();
  }

  public HpnlBuffer getSendBuffer(int bufferId){
    return eqService.getSendBuffer(bufferId);
  }

  public HpnlBuffer getRecvBuffer(int bufferId) {
    return eqService.getRecvBuffer(bufferId);
  }

  public ByteBuffer getRmaBuffer(int rmaBufferId) {
    return eqService.getRmaBufferByBufferId(rmaBufferId);
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

  public void handleCallback(int eventType, int bufferId, int blockBufferSize) {
    Exception e = null;
    if (eventType == EventType.CONNECTED_EVENT) {
      e = executeCallback(connectedCallback, bufferId, 0);
    } else if (eventType == EventType.RECV_EVENT) {
      e = executeCallback(recvCallback, bufferId, blockBufferSize);
    } else if (eventType == EventType.SEND_EVENT) {
      e = executeCallback(sendCallback, bufferId, blockBufferSize);
      putSendBuffer(eqService.getSendBuffer(bufferId));
    } else if (eventType == EventType.READ_EVENT) {
      e = executeCallback(readCallback, bufferId, blockBufferSize);
    }
    if(e != null){
      e.printStackTrace();
    }
  }

  private Exception executeCallback(Handler handler, int bufferId, int blockBufferSize){
    if(handler == null){
      return null;
    }
    try{
      handler.handle(this, bufferId, blockBufferSize);
    }catch(Exception e){
      return e;
    }
    return null;
  }

  private EqService eqService;
  private CqService cqService;
 
  private LinkedBlockingQueue<HpnlBuffer> sendBufferList;

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

  private long nativeHandle;
  private final long nativeEq;
  private int index;
  private long threadId;
}
