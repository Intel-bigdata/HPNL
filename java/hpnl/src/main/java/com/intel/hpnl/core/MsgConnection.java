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

  protected void initialize(long nativeCon) {
    this.init(nativeCon);
  }

  protected int getCqIndexFromNative(long nativeHandle) {
    return this.get_cq_index(nativeHandle);
  }

  protected long getNativeHandle() {
    return this.nativeHandle;
  }

  public int send(int bufferSize, int bufferId) {
    return this.send(bufferSize, bufferId, this.nativeHandle);
  }

  public void releaseRecvBuffer(int bufferId) {
    this.releaseRecvBuffer(bufferId, this.nativeHandle);
  }

  private native void recv(ByteBuffer var1, int var2, long var3);

  private native int send(int var1, int var2, long var3);

  protected native void init(long var1);

  protected native int get_cq_index(long var1);

  public native void finalize();

  private native void releaseRecvBuffer(int var1, long var2);

  private native void deleteGlobalRef(long var1);

  private native void free(long var1);

  protected void doShutdown(boolean proactive) {
    this.service.removeConnection(this.getConnectionId(), this.nativeEq, proactive);
    this.deleteGlobalRef(this.nativeHandle);
    this.free(this.nativeHandle);
  }
}
