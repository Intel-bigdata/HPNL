package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlBuffer;
import java.nio.ByteBuffer;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

public class MemPool {
  private AbstractService service;
  private int initBufferNum;
  private int bufferSize;
  private int nextBufferNum;
  private HpnlBuffer.BufferType type;
  private ConcurrentHashMap<Integer, HpnlBuffer> bufferMap;
  private AtomicInteger seqId;

  public MemPool(AbstractService service, int initBufferNum, int bufferSize, int nextBufferNum, HpnlBuffer.BufferType type) {
    this.service = service;
    this.initBufferNum = initBufferNum;
    this.bufferSize = bufferSize;
    this.nextBufferNum = nextBufferNum;
    this.type = type;
    this.bufferMap = new ConcurrentHashMap();
    this.seqId = new AtomicInteger(0);

    for(int i = 0; i < this.initBufferNum; ++i) {
      this.alloc();
    }

  }

  public void realloc() {
    for(int i = 0; i < this.nextBufferNum; ++i) {
      this.alloc();
    }

  }

  public HpnlBuffer getBuffer(int bufferId) {
    return this.bufferMap.get(bufferId);
  }

  private void alloc() {
    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(this.bufferSize);
    int seq = this.seqId.getAndIncrement();
    HpnlBuffer hpnlBuffer = service.newHpnlBuffer(seq, byteBuffer, type);
    this.bufferMap.put(seq, hpnlBuffer);
    if (this.type == HpnlBuffer.BufferType.SEND) {
      this.service.setSendBuffer(byteBuffer, (long)this.bufferSize, seq);
    } else {
      this.service.setRecvBuffer(byteBuffer, (long)this.bufferSize, seq);
    }
  }
}
