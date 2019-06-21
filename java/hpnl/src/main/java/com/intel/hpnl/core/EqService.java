package com.intel.hpnl.core;

import com.intel.hpnl.api.*;

import java.nio.ByteBuffer;
import java.util.Map;
import java.util.Random;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class EqService extends AbstractService {
  private long nativeHandle;
  private long localEq;
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
    if (this.init(this.workerNum, this.bufferNum, this.server, HpnlConfig.getInstance().getLibfabricProviderName()) == -1) {
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

  @Override
  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    long seqId = this.tryConnect(ip, port, cqIndex);
    if (seqId < 0L) {
      return -1;
    } else {
      if (connectedCallback != null) {
        Handler prv = this.connectedHandlers.putIfAbsent(seqId, connectedCallback);
        if (prv != null) {
          throw new RuntimeException("non-unique id found, " + seqId);
        }
      }

      return 0;
    }
  }

  @Override
  protected void regCon(long eq, long connHandle, String dest_addr, int dest_port, String src_addr, int src_port, long connectId) {
    MsgConnection connection = new MsgConnection(eq, connHandle, this.hpnlService, connectId);
    connection.setAddrInfo(dest_addr, dest_port, src_addr, src_port);
    this.conMap.put(eq, connection);
  }

  @Override
  public void unregCon(long eq) {
    this.conMap.remove(eq);
  }

  protected void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = this.conMap.get(eq);
    if (eventType == 2) {
      long id = connection.getConnectionId();
      Handler connectedHandler = this.connectedHandlers.remove(id);
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

  @Override
  public EventTask getEventTask() {
    return this.eqTask;
  }

  @Override
  protected void setSendBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_send_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  @Override
  protected void setRecvBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_recv_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  @Override
  protected HpnlBuffer newHpnlBuffer(int bufferId, ByteBuffer byteBuffer){
    return new HpnlMsgBuffer(bufferId, byteBuffer);
  }

  public native void shutdown(long eq, long nativeHandle);

  private native long internal_connect(String var1, String var2, int var3, long var4, long nativeHandle);

  public native int wait_eq_event(long nativeHandle);

  public native int add_eq_event(long eq, long nativeHandle);

  public native int delete_eq_event(long eq, long nativeHandle);

  protected native void set_recv_buffer(ByteBuffer buffer, long size, int id, long nativeHandle);

  protected native void set_send_buffer(ByteBuffer buffer, long size, int id, long nativeHandle);

  private native long reg_rma_buffer(ByteBuffer buffer, long size, int id, long nativeHandle);

  private native long reg_rma_buffer_by_address(long var1, long var3, int var5, long nativeHandle);

  private native void unreg_rma_buffer(int var1, long nativeHandle);

  private native long get_buffer_address(ByteBuffer buffer, long nativeHandle);

  private native int init(int workNum, int bufferNum, boolean server, String providerName);

  private native void free(long nativeHandle);

  public native void finalize();

  @Override
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

  @Override
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
