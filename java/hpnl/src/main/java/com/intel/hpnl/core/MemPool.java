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
    this.bufferMap = new ConcurrentHashMap<Integer, HpnlBuffer>();
    this.seqId = new AtomicInteger(0);

    alloc(this.initBufferNum);
  }

  public void realloc() {
    alloc(this.nextBufferNum);
  }
  
  private void alloc(int bufferNum) {
    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(bufferSize*bufferNum);
    int start = 0;
    int end = start;
    for (int i = 0; i < bufferNum; i++) {
      int seq = seqId.getAndIncrement();
      end = (i+1)*bufferSize;
      byteBuffer.position(start);
      byteBuffer.limit(end);
      HpnlBuffer buffer = new HpnlBuffer(seq, byteBuffer.slice());
      start = end;
      bufferMap.put(seq, buffer);
      if (type == Type.SEND) {
        eqService.set_send_buffer(byteBuffer, bufferSize, seq, eqService.getNativeHandle());
      } else {
        eqService.set_recv_buffer(byteBuffer, bufferSize, seq, eqService.getNativeHandle());
      }
    } 
  }

  public HpnlBuffer getBuffer(int bufferId) {
    return bufferMap.get(bufferId); 
  }

  private int initBufferNum;
  private int bufferSize;
  private int nextBufferNum;
  private Type type;
  private EqService eqService;
  private ConcurrentHashMap<Integer, HpnlBuffer> bufferMap;
  private AtomicInteger seqId;
}
