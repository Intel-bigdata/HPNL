package com.intel.hpnl.core;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;
import java.nio.ByteBuffer;

public class MemPool {
  public MemPool(EqService eqService, int initBufferNum, int bufferSize, int nextBufferNum) {
    this.eqService = eqService;
    this.rdmService = null;
    this.initBufferNum = initBufferNum;
    this.bufferSize = bufferSize;
    if (nextBufferNum >= initBufferNum*2) {
      this.nextBufferNum = nextBufferNum; 
    } else {
      this.nextBufferNum = this.initBufferNum*2;
    }
    this.bufferMap = new ConcurrentHashMap<Integer, HpnlBuffer>();
    this.seqId = new AtomicInteger(0);
    for (int i = 0; i < this.initBufferNum; i++) {
      alloc();
    }
  }

  public MemPool(RdmService rdmService, int initBufferNum, int bufferSize, int nextBufferNum) {
    this.eqService = null;
    this.rdmService = rdmService;
    this.initBufferNum = initBufferNum;
    this.bufferSize = bufferSize;
    if (nextBufferNum >= initBufferNum*2) {
      this.nextBufferNum = nextBufferNum; 
    } else {
      this.nextBufferNum = this.initBufferNum*2;
    }
    this.bufferMap = new ConcurrentHashMap<Integer, HpnlBuffer>();
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

  public HpnlBuffer getBuffer(int bufferId) {
    return bufferMap.get(bufferId); 
  }

  private void alloc() {
    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(bufferSize);
    int seq = seqId.getAndIncrement();
    HpnlBuffer buffer = new HpnlBuffer(seq, byteBuffer);
    bufferMap.put(seq, buffer);
    if (eqService!= null) {
      eqService.set_buffer(byteBuffer, bufferSize, seq);
    } else {
      rdmService.set_buffer(byteBuffer, bufferSize, seq);
    }
  }

  private EqService eqService;
  private RdmService rdmService;
  private int initBufferNum;
  private int bufferSize;
  private int nextBufferNum;
  private ConcurrentHashMap<Integer, HpnlBuffer> bufferMap;
  private AtomicInteger seqId;
}
