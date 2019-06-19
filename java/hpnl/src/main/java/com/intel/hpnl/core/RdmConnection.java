package com.intel.hpnl.core;

import com.intel.hpnl.api.AbstractConnection;
import com.intel.hpnl.api.HpnlBuffer;
import com.intel.hpnl.api.HpnlService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.UnknownHostException;
import java.nio.ByteBuffer;

public class RdmConnection extends AbstractConnection {
  private ByteBuffer localName;
  private int localNameLength;
  private boolean server;
  private long nativeConnId;
  private long nativeHandle;

  private static final Logger log = LoggerFactory.getLogger(RdmConnection.class);

  public RdmConnection(long nativeHandle, HpnlService service, boolean server) {
    super(nativeHandle, service, -1L);
    this.localNameLength = this.get_local_name_length(this.nativeHandle);
    this.localName = ByteBuffer.allocateDirect(this.localNameLength);
    this.get_local_name(this.localName, this.nativeHandle);
    this.localName.limit(this.localNameLength);
    this.init(this.nativeHandle);
    this.nativeConnId = get_connection_id(this.nativeHandle);
    this.server = server;
  }

  private native void init(long var1);

  private native void get_local_name(ByteBuffer var1, long nativeHandle);

  private native int get_local_name_length(long nativeHandle);

  private native long get_connection_id(long nativeHandle);

  public native int send(int var1, int var2, long nativeHandle);

  public native int sendTo(int var1, int var2, ByteBuffer var3, long nativeHandle);

  public native int sendBuf(ByteBuffer var1, int var2, long nativeHandle);

  public native int sendBufTo(ByteBuffer var1, int var2, ByteBuffer var3, long nativeHandle);

  private native void releaseRecvBuffer(int var1, long nativeHandle);

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
    if(!server) {
      RdmService.removePortFromRegister(getSrcPort());
    }
    this.free(this.nativeHandle);
  }

  public void pushSendBuffer(HpnlBuffer buffer) {
    ((HpnlRdmBuffer)buffer).setConnectionId(this.getConnectionId());
    super.pushSendBuffer(buffer);
  }

  public void pushRecvBuffer(HpnlBuffer buffer) {
    ((HpnlRdmBuffer)buffer).setConnectionId(this.getConnectionId());
    super.pushRecvBuffer(buffer);
  }

  public void releaseRecvBuffer(int bufferId) {
    this.releaseRecvBuffer(bufferId, this.nativeHandle);
  }

  public int send(int bufferSize, int bufferId) {
    return this.send(bufferSize, bufferId, this.nativeHandle);
  }

  public int sendTo(int bufferSize, int bufferId, ByteBuffer peerName) {
    return this.sendTo(bufferSize, bufferId, peerName, this.nativeHandle);
  }

  public ByteBuffer getLocalName() {
    return this.localName;
  }

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
    String tmp = localIp.replaceAll("\\.", "");
    connectId = Long.valueOf(tmp) + port;
  }
}
