package com.intel.hpnl.core;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.EventTask;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlBuffer;
import com.intel.hpnl.api.HpnlService;
import com.intel.hpnl.core.MemPool.Type;
import java.nio.ByteBuffer;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public abstract class AbstractService {
  protected int workerNum;
  protected int bufferNum;
  protected int bufferSize;
  protected int ioRatio;
  protected boolean server;
  protected HpnlService hpnlService;
  protected Map<Long, Connection> conMap;
  protected boolean stopped;
  private MemPool sendBufferPool;
  private MemPool recvBufferPool;
  private static final Logger log = LoggerFactory.getLogger(AbstractService.class);

  protected AbstractService(int workerNum, int bufferNum, int bufferSize, int ioRatio, boolean server) {
    this.workerNum = workerNum;
    this.bufferNum = bufferNum;
    this.bufferSize = bufferSize;
    this.ioRatio = ioRatio;
    this.server = server;
    this.conMap = new ConcurrentHashMap();
  }

  public void setHpnlService(HpnlService service) {
    this.hpnlService = service;
  }

  protected void initBufferPool(int initBufferNum, int bufferSize, int nextBufferNum) {
    this.sendBufferPool = new MemPool(this, initBufferNum, bufferSize, nextBufferNum, Type.SEND);
    this.recvBufferPool = new MemPool(this, initBufferNum * 2, bufferSize, nextBufferNum * 2, Type.RECV);
  }

  public void reallocBufferPool() {
    this.sendBufferPool.realloc();
    this.recvBufferPool.realloc();
  }

  protected void pushSendBuffer(long eq, int bufferId) {
    Connection connection = this.conMap.get(eq);
    connection.pushSendBuffer(this.sendBufferPool.getBuffer(bufferId));
  }

  protected void pushRecvBuffer(long eq, int bufferId) {
    Connection connection = this.conMap.get(eq);
    connection.pushRecvBuffer(this.recvBufferPool.getBuffer(bufferId));
  }

  protected abstract HpnlBuffer newHpnlBuffer(int bufferId, ByteBuffer byteBuffer);

  protected abstract void setSendBuffer(ByteBuffer buffer, long size, int id);

  protected abstract void setRecvBuffer(ByteBuffer buffer, long size, int id);

  public Connection getConnection(long key) {
    return this.conMap.get(key);
  }

  public HpnlBuffer getSendBuffer(int bufferId) {
    return this.sendBufferPool.getBuffer(bufferId);
  }

  public HpnlBuffer getRecvBuffer(int bufferId) {
    return this.recvBufferPool.getBuffer(bufferId);
  }

  public abstract int connect(String host, String port, int cqIndex, Handler callback);

  protected abstract void regCon(long eq, long connectHandle, String destAddr, int destPort, String srcAddr, int srcPort, long connectId);

  public abstract void unregCon(long eq);

  public abstract void removeConnection(long connectionId, long connHandle, boolean proactive);

  public abstract EventTask getEventTask();

  public abstract void stop();

  protected static long sequenceId() {
    return Math.abs(UUID.randomUUID().getLeastSignificantBits());
  }
}
