package com.intel.hpnl.core;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;
import java.nio.ByteBuffer;

public class MemPool {
  public enum Type {
    SEND, RECV 
  }

  public MemPool(EqService eqService, int initBufferNum, int bufferSize, int nextBufferNum, Type type) {
    this.eqService = eqService;
    this.initBufferNum = initBufferNum;
    this.bufferSize = bufferSize;
    this.nextBufferNum = nextBufferNum;
    this.type = type;
    this.bufferMap = new ConcurrentHashMap<Integer, RdmaBuffer>();
    this.seqId = new AtomicInteger(0);
    for (int i = 0; i < this.initBufferNum; i++) {
      alloc();
    }
  }

  public void realloc() {
    for (int i = 0; i < this.nextBufferNum; i++) {
      alloc();
    }
  }

  public RdmaBuffer getBuffer(int bufferId) {
    return bufferMap.get(bufferId); 
  }

  private void alloc() {
    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(bufferSize);
    int seq = seqId.getAndIncrement();
    RdmaBuffer rdmaBuffer = new RdmaBuffer(seq, byteBuffer);
    bufferMap.put(seq, rdmaBuffer);
    if (type == Type.SEND)
      eqService.set_send_buffer(byteBuffer, bufferSize, seq, eqService.getNativeHandle());
    else
      eqService.set_recv_buffer(byteBuffer, bufferSize, seq, eqService.getNativeHandle());
  }

  private EqService eqService;
  private int initBufferNum;
  private int bufferSize;
  private int nextBufferNum;
  private Type type;
  private ConcurrentHashMap<Integer, RdmaBuffer> bufferMap;
  private AtomicInteger seqId;
}
