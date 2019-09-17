package com.intel.hpnl.core;

import com.intel.hpnl.api.AbstractConnection;
import com.intel.hpnl.api.HpnlBuffer;
import com.intel.hpnl.api.HpnlService;
import java.nio.ByteBuffer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class MsgConnection extends AbstractConnection {

  private long nativeHandle;
  private final long nativeEq;
  private static final Logger log = LoggerFactory.getLogger(MsgConnection.class);

  public MsgConnection(long nativeEq, long nativeCon, HpnlService service, long connectId) {
    super(nativeCon, service, connectId);
    this.nativeEq = nativeEq;
    if (log.isDebugEnabled()) {
      log.debug("connection {} with CQ index {} established.", this.connectId, this.cqIndex);
    }

  }

  @Override
  protected void initialize(long nativeCon) {
    this.init(nativeCon);
  }

  @Override
  protected int getCqIndexFromNative(long nativeHandle) {
    return this.get_cq_index(nativeHandle);
  }

  @Override
  protected long getNativeHandle() {
    return this.nativeHandle;
  }

  public boolean isServer() {
    return false;
  }

  @Override
  public int sendBufferToAddress(HpnlBuffer buffer, int bufferSize, long peerAddress){ return -1;}

  @Override
  public int sendBufferToId(HpnlBuffer buffer, int bufferSize, long peerConnectionId){return -1;}

  @Override
  public int sendBufferToId(ByteBuffer buffer, int bufferSize, long peerConnectionId){return -1;}

  @Override
  public int sendBuffer(ByteBuffer buffer, int bufferSize){ return -1;}

  @Override
  public long resolvePeerName(ByteBuffer peerName) {
    return 0;
  }

  @Override
  public int sendBuffer(HpnlBuffer buffer, int bufferSize){return -1;}

  @Override
  public void reclaimRecvBuffer(int bufferId) {
    this.releaseRecvBuffer(bufferId, this.nativeHandle);
  }

  @Override
  public void putProviderAddress(long connectionId, long peerAddress){}

  @Override
  public long getProviderAddress(long connectionId){return -1;}

  @Override
  public void putPeerAddress(long connectId, Object[] address){}

  @Override
  public Object[] getPeerAddress(long connectId){return null;}

  private native void recv(ByteBuffer buffer, int mid, long nativeHandle);

  private native int send(int bufferSize, int bufferId, long nativeHandle);

  protected native void init(long nativeHandle);

  protected native int get_cq_index(long nativeHandle);

  public native void finalize();

  private native void releaseRecvBuffer(int bufferId, long nativeHandle);

  private native void deleteGlobalRef(long nativeHandle);

  private native void free(long nativeHandle);

  @Override
  protected void doShutdown(boolean proactive) {
    this.service.removeNativeConnection(this.getConnectionId(), this.nativeEq, proactive);
    this.deleteGlobalRef(this.nativeHandle);
    this.free(this.nativeHandle);
  }
}
