package com.intel.hpnl.core;

import com.intel.hpnl.api.AbstractConnection;
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
  protected void addTask(int eventType, int bufferId, int bufferSize){}

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
  public int send(int bufferSize, int bufferId) {
    return this.send(bufferSize, bufferId, this.nativeHandle);
  }

  @Override
  public void releaseRecvBuffer(int bufferId) {
    this.releaseRecvBuffer(bufferId, this.nativeHandle);
  }

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
