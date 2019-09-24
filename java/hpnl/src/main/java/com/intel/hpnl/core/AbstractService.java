package com.intel.hpnl.core;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.EventTask;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlBuffer;
import com.intel.hpnl.api.HpnlService;
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
  protected int recvBufferNum;
  protected boolean server;
  protected HpnlService hpnlService;
  protected Map<Long, Connection> conMap;
  protected boolean stopped;
  private MemPool sendBufferPool;
  private MemPool recvBufferPool;
  private static final Logger log = LoggerFactory.getLogger(AbstractService.class);

  protected AbstractService(int workerNum, int bufferNum, int recvBufferNum, int bufferSize, boolean server) {
    this.workerNum = workerNum;
    this.bufferNum = bufferNum;
    this.bufferSize = bufferSize;
    this.recvBufferNum = recvBufferNum;
    this.server = server;
    this.conMap = new ConcurrentHashMap();
  }

  public void setHpnlService(HpnlService service) {
    this.hpnlService = service;
  }

  protected void initBufferPool(int bufferNum, int recvBufferNum, int bufferSize) {
    this.sendBufferPool = new MemPool(this, bufferNum, bufferSize, bufferNum, HpnlBuffer.BufferType.SEND);
    this.recvBufferPool = new MemPool(this, recvBufferNum, bufferSize, recvBufferNum, HpnlBuffer.BufferType.RECV);
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

  protected abstract HpnlBuffer newHpnlBuffer(int bufferId, ByteBuffer byteBuffer, HpnlBuffer.BufferType type);

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

  public abstract int connect(String host, String port, int cqIndex, Handler connectedCallback, Handler recvCallback);

  protected abstract void regCon(long eq, long connectHandle, String destAddr, int destPort,
                                 String srcAddr, int srcPort, long connectId);

  public abstract void unregCon(long eq);

  public abstract void removeConnection(long connectionId, long connHandle, boolean proactive);

  public abstract EventTask getEventTask(int cqIndex);

  public abstract void stop();

  protected static long sequenceId() {
    return Math.abs(UUID.randomUUID().getLeastSignificantBits());
  }
}
