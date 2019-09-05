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
    if(type == HpnlBuffer.BufferType.SEND) {
      this.seqId = new AtomicInteger(HpnlBuffer.SEND_BUFFER_ID_START); //start from 1. 0 will not be reclaimed.
    }else if(type == HpnlBuffer.BufferType.RECV){
      this.seqId = new AtomicInteger(HpnlBuffer.RECV_BUFFER_ID_START);
    }else{
      throw new IllegalArgumentException("unsupported buffer type, "+type);
    }

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
      if(seq >= HpnlBuffer.RECV_BUFFER_ID_START){
        throw new IllegalStateException("send buffer id should not exceed. "+seq);
      }
      this.service.setSendBuffer(byteBuffer, this.bufferSize, seq);
    } else {
      if(seq < 0){
        throw new IllegalStateException("recv buffer id should be negative. "+seq);
      }
      this.service.setRecvBuffer(byteBuffer, this.bufferSize, seq);
    }
  }
}
