package com.intel.hpnl.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Java representative of native connection
 */
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
  private List<Handler> shutdownCallbacks = new ArrayList<>();
  private Handler generalCallback;

  private long nativeHandle;
  private final long nativeEq;

  private int cqIndex;
  private final long connectId;

  private static final Logger log = LoggerFactory.getLogger(Connection.class);

  /**
   * instantiated when connection is to be registered in connection map. The registration
   * is from JNI call.
   * @param nativeEq
   * @param nativeCon
   * @param service
   * @param cqService
   * @param connectId
   */
  public Connection(long nativeEq, long nativeCon, EqService service, CqService cqService, long connectId) {
    this.service = service;
    this.cqService = cqService;
    this.sendBufferList = new LinkedBlockingQueue<>();
    this.nativeEq = nativeEq;
    init(nativeCon);
    cqIndex = get_cq_index(this.nativeHandle);
    connected = true;
    if(connectId == 0){
      this.connectId = service.getNewConnectionId();
    }else {
      this.connectId = connectId;
    }
    shutdownCallbacks.add(new InternalShutdownCallback());
    if(log.isDebugEnabled()) {
      log.debug("connection {} with CQ index {} established.", this.connectId, cqIndex);
    }
  }

  /**
   * shutdown connection from Java side by,
   * - shutdown native connection
   * - delete EQ event
   * - delete global references
   * - free JNI resources
   */
  public void shutdown(){
    shutdown(true);
  }

  /**
   * shutdown proactively or passively
   * @param proactive
   */
  private void shutdown(boolean proactive){
    if(!connected){
      return;
    }
    synchronized (this) {
      if (!connected) {
        return;
      }
      this.service.shutdown(nativeEq, service.getNativeHandle());
      if (proactive) {
        this.service.delete_eq_event(nativeEq, service.getNativeHandle());
      }
      deleteGlobalRef(this.nativeHandle);
      //call un-register again in case of JNI event handling of un-register failed or not executed
      this.service.unregCon(nativeEq);
      free(nativeHandle);
      connected = false;
      if(log.isDebugEnabled()) {
        log.debug("connection {} with CQ index {} closed.", connectId, cqIndex);
      }
    }
  }

  /**
   * send buffer identified by <code>rdmaBufferId</code>
   * @param blockBufferSize
   * @param rdmaBufferId
   * @return
   */
  public int send(int blockBufferSize, int rdmaBufferId) {
    return send(blockBufferSize, rdmaBufferId, this.nativeHandle);
  }

  /**
   * RDMA read
   * @param rdmaBufferId
   * @param localOffset
   * @param len
   * @param remoteAddr
   * @param remoteMr
   * @return
   */
  public int read(int rdmaBufferId, int localOffset, long len, long remoteAddr, long remoteMr) {
    return read(rdmaBufferId, localOffset, len, remoteAddr, remoteMr, this.nativeHandle);
  }

  /**
   * release receive buffer proactively from Java
   * @param rdmaBufferId
   */
  public void releaseRecvBuffer(int rdmaBufferId){
    releaseRecvBuffer(rdmaBufferId, nativeHandle);
  }

  //native methods
  private native void recv(ByteBuffer buffer, int id, long nativeHandle);
  private native int send(int blockBufferSize, int rdmaBufferId, long nativeHandle);
  private native int read(int rdmaBufferId, int localOffset, long len, long remoteAddr, long remoteMr, long nativeHandle);
  private native void init(long eq);
  private native int get_cq_index(long nativeHandle);
  public native void finalize();
  private native void releaseRecvBuffer(int rdmaBufferId, long nativeHandle);
  private native void deleteGlobalRef(long nativeHandle);
  private native void free(long nativeHandle);

  //getters and putters for callbacks
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

  public List<Handler> getShutdownCallbacks() {
    return shutdownCallbacks;
  }

  public void addShutdownCallback(Handler callback) {
    shutdownCallbacks.add(callback);
  }

  public void setGeneralEventCallback(Handler callback){
    this.generalCallback = callback;
  }

  public Handler getGeneralCallback() {
    return generalCallback;
  }


  /**
   * put send buffer back after sendCallback is invoked
   * @param buffer
   */
  public void putSendBuffer(RdmaBuffer buffer) {
    try {
      sendBufferList.put(buffer);
    } catch (InterruptedException e) {
      log.error("putSendBuffer interrupted.", e);
    }
  }

  /**
   * take send buffer from connection's pool.
   * wait for buffer's availability if <code>wait</code> is true
   * @param wait
   * @return
   */
  public RdmaBuffer takeSendBuffer(boolean wait) {
    if (wait) {
      try {
        return sendBufferList.take();
      } catch (InterruptedException e) {
        log.error("putSendBuffer interrupted.", e);
      }
      return null;
    }
    return sendBufferList.poll();
  }

  /**
   * get send buffer from global send buffer pool
   * @param rdmaBufferId
   * @return
   */
  public RdmaBuffer getSendBuffer(int rdmaBufferId){
    return service.getSendBuffer(rdmaBufferId);
  }

  /**
   * get receive buffer from global receive buffer pool
   * @param rdmaBufferId
   * @return
   */
  public RdmaBuffer getRecvBuffer(int rdmaBufferId) {
    return service.getRecvBuffer(rdmaBufferId);
  }

  /**
   * get RDMA buffer from global RDMA buffer pool
   * @param rmaBufferId
   * @return
   */
  public ByteBuffer getRmaBuffer(int rmaBufferId) {
    return service.getRmaBufferByBufferId(rmaBufferId);
  }

  /**
   * set address information when connection is registered.
   * @param destAddr
   * @param destPort
   * @param srcAddr
   * @param srcPort
   */
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

  public long getConnectionId() {
    return connectId;
  }

  /**
   * handle callback of all connection's events. Invoked from JNI
   * @param eventType
   * @param rdmaBufferId
   * @param blockBufferSize
   * @return
   */
  public int handleCallback(int eventType, int rdmaBufferId, int blockBufferSize) {
    int e;
    switch (eventType){
      case EventType.RECV_EVENT:
        e = executeCallback(recvCallback, rdmaBufferId, blockBufferSize);
        //receive buffer is released either proactively inside handler or passively in JNI
        //depending on result of handler.
        break;
      case EventType.SEND_EVENT:
        e = executeCallback(sendCallback, rdmaBufferId, blockBufferSize);
        if(e == Handler.RESULT_DEFAULT) {
          //TODO: get buffer from connection's pool
          putSendBuffer(service.getSendBuffer(rdmaBufferId));
        }
        break;
      case EventType.READ_EVENT:
        e = executeCallback(readCallback, rdmaBufferId, blockBufferSize);
        break;
      case EventType.CONNECTED_EVENT:
        e = executeCallback(connectedCallback, rdmaBufferId, 0);
        break;
      case EventType.SHUTDOWN:
        e = executeCallbacks(shutdownCallbacks, rdmaBufferId, 0);
        break;
      default:
        e = Handler.RESULT_DEFAULT;
    }
    //general event callback
    executeCallback(generalCallback, -1, -1);
    return e;
  }

  private int executeCallback(Handler handler, int rdmaBufferId, int blockBufferSize){
    if(handler == null){
      return Handler.RESULT_DEFAULT;
    }
    try{
      return handler.handle(this, rdmaBufferId, blockBufferSize);
    }catch(Throwable e){
      log.error("failed to execute callback "+handler, e);
      return Handler.RESULT_DEFAULT;
    }
  }

  private int executeCallbacks(List<Handler> callbacks, int rdmaBufferId, int bufferSize){
    int ret = Handler.RESULT_DEFAULT;
    for (Handler callback : callbacks) {
      int tmp = executeCallback(callback, rdmaBufferId, bufferSize);
      if(tmp == Handler.RESULT_BUF_RELEASED){
        ret = tmp;
      }
    }
    return ret;
  }

  /**
   * release connection resources passively after shutdown event being polled from native code.
   *
   */
  private class InternalShutdownCallback implements Handler{
    @Override
    public int handle(Connection connection, int rdmaBufferId, int blockBufferSize){
      shutdown(false);
      return Handler.RESULT_DEFAULT;
    }
  }
}
