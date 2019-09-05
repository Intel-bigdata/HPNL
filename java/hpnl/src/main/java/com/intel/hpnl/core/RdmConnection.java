package com.intel.hpnl.core;

import com.intel.hpnl.api.AbstractConnection;
import com.intel.hpnl.api.HpnlBuffer;
import com.intel.hpnl.api.HpnlService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class RdmConnection extends AbstractConnection{
  private ByteBuffer localName;
  private int localNameLength;
  private boolean server;
  private long nativeConnId;
  private long nativeHandle;
  private int ctxNum;

  private BlockingQueue<Runnable> eventQueue;
  private Map<Long, ByteBuffer> peerMap;
  private Map<Long, Object[]> addressMap;
  private Map<Integer, HpnlBuffer> globalBufferMap;
  private Queue<Integer> ctxIdQueue;

  private static final Logger log = LoggerFactory.getLogger(RdmConnection.class);

  public RdmConnection(long nativeHandle, HpnlService service, boolean server, int ctxNum) {
    super(nativeHandle, service, -1L);
    this.localNameLength = this.get_local_name_length(this.nativeHandle);
    this.localName = ByteBuffer.allocateDirect(this.localNameLength);
    this.get_local_name(this.localName, this.nativeHandle);
    this.localName.limit(this.localNameLength);
    this.init(this.nativeHandle);
    this.nativeConnId = get_connection_id(this.nativeHandle);
    this.server = server;
    this.ctxNum = ctxNum;
    ctxIdQueue = new ConcurrentLinkedQueue<>();
    for(int i=0; i<ctxNum; i++){
      ctxIdQueue.offer(Integer.valueOf(i));
    }
    globalBufferMap = new ConcurrentHashMap<>();
    if(server){
      peerMap = new ConcurrentHashMap<>();
      addressMap = new ConcurrentHashMap<>();
    }
  }

  private native void init(long nativeHandle);

  private native void get_local_name(ByteBuffer localName, long nativeHandle);

  private native int get_local_name_length(long nativeHandle);

  private native long get_connection_id(long nativeHandle);

  public native int send(int size, int id, long nativeHandle);

  public native int sendTo(int size, int id, ByteBuffer peer, long nativeHandle);

  public native int sendBuf(ByteBuffer buffer, int bufferId, int ctxId, int size, long nativeHandle);

  public native int sendBufTo(ByteBuffer buffer, int bufferId, int ctxId, int size, ByteBuffer peer, long nativeHandle);

  private native void releaseRecvBuffer(int id, long nativeHandle);

  private native void deleteGlobalRef(long nativeHandle);

  private native void free(long nativeHandle);

  protected void initialize(long nativeCon) {
    this.init(nativeCon);
  }

  protected int getCqIndexFromNative(long nativeHandle) {
    return 0;
  }

  protected long getNativeHandle() {
    return this.nativeHandle;
  }

  @Override
  public void setEventQueue(BlockingQueue<Runnable> eventQueue) {
    this.eventQueue = eventQueue;
  }

  /**
   * add task to event queue owned by one thread
   * @param task
   */
  @Override
  protected void addTask(Runnable task){
    try {
      eventQueue.put(task);
    }catch (InterruptedException e){
      throw new RuntimeException(e);
    }
  }

  @Override
  protected void doShutdown(boolean proactive) {
    this.service.removeNativeConnection(nativeConnId, this.nativeHandle, proactive);
    this.deleteGlobalRef(this.nativeHandle);
    ctxIdQueue.clear();
    globalBufferMap.clear();
    if(server){
      peerMap.clear();
      addressMap.clear();
    }
//    this.free(this.nativeHandle);
  }

  @Override
  public void pushSendBuffer(HpnlBuffer buffer) {
    buffer.setConnectionId(this.getConnectionId());
    ((HpnlRdmBuffer)buffer).setConnection(this);
    super.pushSendBuffer(buffer);
  }

  @Override
  public void pushRecvBuffer(HpnlBuffer buffer) {
    buffer.setConnectionId(this.getConnectionId());
    ((HpnlRdmBuffer)buffer).setConnection(this);
    super.pushRecvBuffer(buffer);
  }

  @Override
  public void reclaimRecvBuffer(int bufferId) {
    this.releaseRecvBuffer(bufferId, this.nativeHandle);
  }

  @Override
  protected void reclaimGlobalBuffer(int bufferId, int ctxId){
    globalBufferMap.get(bufferId).release();
    ctxIdQueue.offer(ctxId);
  }

  @Override
  protected void reclaimCtxId(int ctxId) {
    if(ctxId >= ctxNum){
      throw new IllegalArgumentException("invalid context id, "+ctxId);
    }
    ctxIdQueue.offer(ctxId);
  }

  @Override
  public int sendBufferTo(HpnlBuffer buffer, int bufferSize, long peerConnectionId) {
    return this.sendBufferTo(buffer, bufferSize, peerMap.get(Long.valueOf(peerConnectionId)));
  }

  @Override
  public int sendBufferTo(HpnlBuffer buffer, int bufferSize, ByteBuffer peerName) {
    int bufferId = buffer.getBufferId();
    if(bufferId >  0){
      return this.sendTo(bufferSize, bufferId, peerName, this.nativeHandle);
    }

    if(!globalBufferMap.containsKey(bufferId)){
      globalBufferMap.put(Integer.valueOf(bufferId), buffer);
    }
    Integer ctxId = ctxIdQueue.poll();
    buffer.setConnectionId(getConnectionId());
    return this.sendBufTo(buffer.getRawBuffer(), bufferId, ctxId==null?-1:ctxId.intValue(), bufferSize,
            peerName, this.nativeHandle);
  }

  @Override
  public int sendBuffer(HpnlBuffer buffer, int bufferSize) {
    int bufferId = buffer.getBufferId();
//    log.info("buffer id: {}, {}", bufferId, bufferSize);
    if(bufferId > 0 ){
      return this.send(bufferSize, bufferId, this.nativeHandle);
    }
    // for non registered buffer
    if(!globalBufferMap.containsKey(bufferId)){
      globalBufferMap.put(Integer.valueOf(bufferId), buffer);
    }
    Integer ctxId = ctxIdQueue.poll();
    buffer.setConnectionId(getConnectionId());
    return this.sendBuf(buffer.getRawBuffer(), buffer.getBufferId(), ctxId==null?-1:ctxId.intValue(),
            bufferSize, this.nativeHandle);

  }

  /**
   * send buffer without cache
   * @param buffer
   * @param bufferSize
   * @return
   */
  @Override
  public int sendBuffer(ByteBuffer buffer, int bufferSize) {
    return this.sendBuf(buffer, -1, -1,
            bufferSize, this.nativeHandle);
  }

  @Override
  public int sendBufferTo(ByteBuffer buffer, int bufferSize, long peerConnectionId) {
    return this.sendBufTo(buffer, -1, -1,
            bufferSize, peerMap.get(Long.valueOf(peerConnectionId)), this.nativeHandle);
  }

  @Override
  public ByteBuffer getLocalName() {
    return this.localName;
  }

  @Override
  public void putPeerName(long connectionId, ByteBuffer peer) {
    peerMap.put(Long.valueOf(connectionId), peer);
  }

  @Override
  public ByteBuffer getPeerName(long connectionId){
    return peerMap.get(Long.valueOf(connectionId));
  }

  @Override
  public void putPeerAddress(long connectId, Object[] address){
    addressMap.put(Long.valueOf(connectId), address);
  }

  @Override
  public Object[] getPeerAddress(long connectId){
    return addressMap.get(connectId);
  }

  @Override
  public boolean isServer() {
    return server;
  }

  /**
   * globally unique id based on IP and port
   * @param localIp
   * @param port
   */
  public void setConnectionId(String localIp, int port){
    if(!Utils.isIp(localIp)) {
      try {
        localIp = Utils.getIpFromHostname(localIp);
      }catch(UnknownHostException e){
        log.error("failed to resolve "+localIp, e);
        throw new RuntimeException(e);
      }
    }
    String tmp = localIp.replaceAll("\\.", "") + port;
    connectId = Long.valueOf(tmp);
  }
}
