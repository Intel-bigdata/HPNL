package com.intel.hpnl.core;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.EventTask;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlFactory;
import java.nio.ByteBuffer;
import java.util.Map;
import java.util.Random;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicLong;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class EqService extends AbstractService {
  private long nativeHandle;
  private long localEq;
  private volatile CountDownLatch connectLatch;
  private EventTask eqTask;
  private Map<Long, Handler> connectedHandlers;
  private CqService cqService;
  private AtomicLong nextConnectId;
  private static final Logger log = LoggerFactory.getLogger("com.intel.hpnl.core.EqService");

  protected EqService(int workerNum, int bufferNum, int bufferSize, boolean server) {
    super(workerNum, bufferNum, bufferSize, server);
    this.connectedHandlers = new ConcurrentHashMap();
    this.nextConnectId = new AtomicLong();
    this.eqTask = new EqService.EqTask();
    this.nextConnectId.set((new Random()).nextLong());
  }

  public EqService(int workerNum, int bufferNum, int bufferSize) {
    this(workerNum, bufferNum, bufferSize, false);
  }

  public EqService init() {
    if (this.init(this.workerNum, this.bufferNum, this.server, HpnlFactory.getLibfabricProviderName()) == -1) {
      return null;
    } else {
      this.initBufferPool(this.bufferNum, this.bufferSize, this.bufferNum);
      return this;
    }
  }

  protected long tryConnect(String ip, String port, int cqIndex) {
    long id = sequenceId();
    this.localEq = this.internal_connect(ip, port, cqIndex, id, this.nativeHandle);
    if (this.localEq == -1L) {
      return -1L;
    } else {
      this.add_eq_event(this.localEq, this.nativeHandle);
      return id;
    }
  }

  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    long seqId = this.tryConnect(ip, port, cqIndex);
    if (seqId < 0L) {
      return -1;
    } else {
      if (connectedCallback != null) {
        Handler prv = (Handler)this.connectedHandlers.putIfAbsent(seqId, connectedCallback);
        if (prv != null) {
          throw new RuntimeException("non-unique id found, " + seqId);
        }
      }

      return 0;
    }
  }

  protected void regCon(long eq, long connHandle, String dest_addr, int dest_port, String src_addr, int src_port, long connectId) {
    MsgConnection connection = new MsgConnection(eq, connHandle, this.hpnlService, connectId);
    connection.setAddrInfo(dest_addr, dest_port, src_addr, src_port);
    this.conMap.put(eq, connection);
  }

  public void unregCon(long eq) {
    this.conMap.remove(eq);
  }

  protected void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = (Connection)this.conMap.get(eq);
    if (eventType == 2) {
      long id = connection.getConnectionId();
      Handler connectedHandler = (Handler)this.connectedHandlers.remove(id);
      if (connectedHandler != null) {
        connectedHandler.handle(connection, 0, 0);
      }
    }

  }

  public long getNativeHandle() {
    return this.nativeHandle;
  }

  public void setCqService(CqService cqService) {
    this.cqService = cqService;
  }

  public int getWorkerNum() {
    return this.workerNum;
  }

  public EventTask getEventTask() {
    return this.eqTask;
  }

  protected void setSendBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_send_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  protected void setRecvBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_recv_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  public native void shutdown(long var1, long var3);

  private native long internal_connect(String var1, String var2, int var3, long var4, long var6);

  public native int wait_eq_event(long var1);

  public native int add_eq_event(long var1, long var3);

  public native int delete_eq_event(long var1, long var3);

  protected native void set_recv_buffer(ByteBuffer var1, long var2, int var4, long var5);

  protected native void set_send_buffer(ByteBuffer var1, long var2, int var4, long var5);

  private native long reg_rma_buffer(ByteBuffer var1, long var2, int var4, long var5);

  private native long reg_rma_buffer_by_address(long var1, long var3, int var5, long var6);

  private native void unreg_rma_buffer(int var1, long var2);

  private native long get_buffer_address(ByteBuffer var1, long var2);

  private native int init(int var1, int var2, boolean var3, String var4);

  private native void free(long var1);

  public native void finalize();

  public void stop() {
    if(!stopped) {
      synchronized (this) {
        if(!stopped) {
          this.eqTask.stop();
          this.waitToComplete();
          this.delete_eq_event(this.localEq, this.nativeHandle);
          this.free(this.nativeHandle);
          stopped = true;
        }
      }
    }
  }

  private void waitToComplete() {
    try {
      this.eqTask.waitToComplete();
    } catch (InterruptedException var5) {
      log.error("EQ task interrupted when wait its completion", var5);
    } finally {
      log.info("EQ task stopped? {}", this.eqTask.isStopped());
    }

  }

  public void removeConnection(long connectionId, long connEq, boolean proactive) {
    this.shutdown(connEq, this.getNativeHandle());
    if (proactive) {
      this.delete_eq_event(connEq, this.getNativeHandle());
    }

    this.unregCon(connEq);
  }

  public long getNewConnectionId() {
    return this.nextConnectId.incrementAndGet();
  }

  protected class EqTask extends EventTask {
    protected EqTask() {
    }

    public void waitEvent() {
      if (EqService.this.wait_eq_event(EqService.this.nativeHandle) == -1) {
        EqService.log.warn("wait or process EQ event error, ignoring");
      }

    }

    protected Logger getLogger() {
      return EqService.log;
    }

    protected void cleanUp() {
      EqService.log.info("close and remove all connections");
      EqService.this.conMap.clear();
    }
  }
}
