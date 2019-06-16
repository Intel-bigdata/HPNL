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
  protected boolean server;
  protected HpnlService hpnlService;
  protected Map<Long, Connection> conMap;
  private MemPool sendBufferPool;
  private MemPool recvBufferPool;
  private static final Logger log = LoggerFactory.getLogger(AbstractService.class);

  protected AbstractService(int workerNum, int bufferNum, int bufferSize, boolean server) {
    this.workerNum = workerNum;
    this.bufferNum = bufferNum;
    this.bufferSize = bufferSize;
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
    Connection connection = (Connection)this.conMap.get(eq);
    connection.pushSendBuffer(this.sendBufferPool.getBuffer(bufferId));
  }

  protected void pushRecvBuffer(long eq, int bufferId) {
    Connection connection = (Connection)this.conMap.get(eq);
    connection.pushRecvBuffer(this.recvBufferPool.getBuffer(bufferId));
  }

  protected abstract void setSendBuffer(ByteBuffer var1, long var2, int var4);

  protected abstract void setRecvBuffer(ByteBuffer var1, long var2, int var4);

  public Connection getConnection(long key) {
    return (Connection)this.conMap.get(key);
  }

  public HpnlBuffer getSendBuffer(int bufferId) {
    return this.sendBufferPool.getBuffer(bufferId);
  }

  public HpnlBuffer getRecvBuffer(int bufferId) {
    return this.recvBufferPool.getBuffer(bufferId);
  }

  public abstract int connect(String var1, String var2, int var3, Handler var4);

  protected abstract void regCon(long var1, long var3, String var5, int var6, String var7, int var8, long var9);

  public abstract void unregCon(long var1);

  public abstract void removeConnection(long var1, boolean var3);

  public abstract EventTask getEventTask();

  public abstract void stop();

  protected static long sequenceId() {
    return Math.abs(UUID.randomUUID().getLeastSignificantBits());
  }
}
