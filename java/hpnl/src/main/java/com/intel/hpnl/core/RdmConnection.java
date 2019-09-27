package com.intel.hpnl.core;

import com.intel.hpnl.api.AbstractConnection;
import com.intel.hpnl.api.HpnlBuffer;
import com.intel.hpnl.api.HpnlService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.Queue;

public class RdmConnection extends AbstractConnection{
  private ByteBuffer localName;
  private int localNameLength;
  private boolean server;
  private long nativeConnId;
  private long nativeHandle;
  private int ctxNum;

  private Map<Integer, HpnlBuffer> globalBufferMap = new HashMap<>();

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
    ctxIdQueue = new LinkedList<>();
    for(int i=0; i<ctxNum; i++){
      ctxIdQueue.offer(Integer.valueOf(i));
    }
  }

  private native void init(long nativeHandle);

  private native void get_local_name(ByteBuffer localName, long nativeHandle);

  private native int get_local_name_length(long nativeHandle);

  private native long get_connection_id(long nativeHandle);

  private native long resolve_peer_name(ByteBuffer peerName, long nativeHandle);

  private native int send(int size, int id, long nativeHandle);

  private native int sendRequest(int size, int id, long nativeHandle);

  private native int sendTo(int size, int id, long peerAddress, long nativeHandle);

  private native int sendBuf(ByteBuffer buffer, int bufferId, int ctxId, int size, long nativeHandle);

  private native int sendBufWithRequest(ByteBuffer buffer, int bufferId, int ctxId, int size, long nativeHandle);

  private native int sendBufTo(ByteBuffer buffer, int bufferId, int ctxId, int size, long peerAddress, long nativeHandle);

  private native void releaseRecvBuffer(int id, long nativeHandle);

  private native void adjustSendTarget(int sendCtxId, long nativeHandle);

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
  protected void doShutdown(boolean proactive) {
    this.service.removeNativeConnection(nativeConnId, this.nativeHandle, proactive);
    this.deleteGlobalRef(this.nativeHandle);
    ctxIdQueue.clear();
    globalBufferMap.clear();
//    this.free(this.nativeHandle);
  }

  @Override
  public void pushSendBuffer(HpnlBuffer buffer) {
    ((HpnlRdmBuffer)buffer).setConnection(this);
    super.pushSendBuffer(buffer);
  }

  @Override
  public void pushRecvBuffer(HpnlBuffer buffer) {
    ((HpnlRdmBuffer)buffer).setConnection(this);
    super.pushRecvBuffer(buffer);
  }

  @Override
  public void reclaimRecvBuffer(int bufferId) {
    this.releaseRecvBuffer(bufferId, this.nativeHandle);
  }

  @Override
  protected void reclaimGlobalBuffer(int bufferId, int ctxId){
    HpnlBuffer hpnlBuffer = globalBufferMap.remove(bufferId);
    if (hpnlBuffer == null) {
      throw new IllegalStateException("failed to reclaim send buffer (not found) with id: " + bufferId);
    }
    hpnlBuffer.release();
    if(ctxId >= 0) {
      if(ctxId >= ctxNum){
        throw new IllegalArgumentException("invalid context id, "+ctxId);
      }
      ctxIdQueue.offer(ctxId);
    }
  }

  @Override
  public int sendBuffer(HpnlBuffer buffer, int bufferSize) {
    int bufferId = buffer.getBufferId();
//    log.info("buffer id: {}, {}", bufferId, bufferSize);
    if(bufferId > 0 ){
      return this.send(bufferSize, bufferId, this.nativeHandle);
    }
    // for non registered buffer
    globalBufferMap.put(bufferId, buffer);
    Integer ctxId = ctxIdQueue.poll();
    return this.sendBuf(buffer.getRawBuffer(), buffer.getBufferId(), ctxId == null ? -1 : ctxId.intValue(),
              bufferSize, this.nativeHandle);
  }

  @Override
  public int sendConnectRequest(HpnlBuffer buffer, int bufferSize){
    int bufferId = buffer.getBufferId();
//    log.info("buffer id: {}, {}", bufferId, bufferSize);
    if(bufferId > 0 ){
      return this.sendRequest(bufferSize, bufferId, this.nativeHandle);
    }
    // for non registered buffer
    globalBufferMap.put(bufferId, buffer);
    Integer ctxId = ctxIdQueue.poll();
    return this.sendBufWithRequest(buffer.getRawBuffer(), buffer.getBufferId(), ctxId == null ? -1 : ctxId.intValue(),
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
  public ByteBuffer getLocalName() {
    return this.localName;
  }

  @Override
  public long resolvePeerName(ByteBuffer peerName) {
    return resolve_peer_name(peerName, this.nativeHandle);
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

  @Override
  public void adjustSendTarget(int sendCtxId){
    adjustSendTarget(sendCtxId, nativeHandle);
  }
}
