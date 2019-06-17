package com.intel.hpnl.core;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.EventTask;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlFactory;
import java.nio.ByteBuffer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RdmService extends AbstractService {
  private EventTask task;
  private long nativeHandle;
  private static final Logger log = LoggerFactory.getLogger(RdmService.class);

  public RdmService(int workNum, int bufferNum, int bufferSize) {
    this(workNum, bufferNum, bufferSize, false);
  }

  protected RdmService(int workNum, int bufferNum, int bufferSize, boolean server) {
    super(workNum, bufferNum, bufferSize, server);
  }

  public RdmService init() {
    this.init(this.bufferNum, this.server, HpnlFactory.getLibfabricProviderName());
    this.initBufferPool(this.bufferNum, this.bufferSize, this.bufferNum);
    this.task = new RdmService.RdmTask();
    return this;
  }

  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    Connection conn = this.conMap.get(this.get_con(ip, port, this.nativeHandle));
    connectedCallback.handle(conn, -1, -1);
    return 1;
  }

  protected void regCon(long key, long connHandle, String dest_addr, int dest_port, String src_addr, int src_port, long connectId) {
    RdmConnection con = new RdmConnection(connHandle, this.hpnlService, this.server);
    con.setAddrInfo(dest_addr, dest_port, src_addr, src_port);
    this.conMap.put(connHandle, con);
  }

  public void unregCon(long connHandle) {
    this.conMap.remove(connHandle);
  }

  public void removeConnection(long connectionId, long connHandle, boolean proactive) {
    remove_connection(connectionId, nativeHandle);
    this.conMap.remove(connHandle);
  }

  public EventTask getEventTask() {
    return this.task;
  }

  public void stop() {
    if(!stopped) {
      synchronized (this) {
        if (!stopped) {
          this.task.stop();
          this.waitToComplete();
          this.free(this.nativeHandle);
          stopped = true;
        }
      }
    }
  }

  private void waitToComplete() {
    try {
      this.task.waitToComplete();
    } catch (InterruptedException var5) {
      log.error("EQ task interrupted when wait its completion", var5);
    } finally {
      log.info("EQ task stopped? {}", this.task.isStopped());
    }

  }

  public long getNativeHandle() {
    return this.nativeHandle;
  }

  protected void setSendBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_send_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  protected void setRecvBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_recv_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  private native int init(int var1, boolean var2, String nativeHandle);

  protected native long listen(String var1, String var2, long nativeHandle);

  private native long get_con(String var1, String var2, long nativeHandle);

  private native int wait_event(long nativeHandle);

  public native void set_recv_buffer(ByteBuffer var1, long var2, int var4, long nativeHandle);

  public native void set_send_buffer(ByteBuffer var1, long var2, int var4, long nativeHandle);

  public native void remove_connection(long id, long nativeHandle);

  private native void free(long nativeHandle);

  protected class RdmTask extends EventTask {
    protected RdmTask() {
    }

    public void waitEvent() {
      if (RdmService.this.wait_event(RdmService.this.nativeHandle) == -1) {
        RdmService.log.warn("wait or process event error, ignoring");
      }

    }

    protected Logger getLogger() {
      return RdmService.log;
    }

    protected void cleanUp() {
      RdmService.log.info("close and remove all connections");
      RdmService.this.conMap.clear();
    }
  }
}
