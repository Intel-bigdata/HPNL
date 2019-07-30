// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

package com.intel.hpnl.core;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;
import java.nio.ByteBuffer;

public class MemPool {
  public MemPool(MemoryService service, int initBufferNum, int bufferSize, int nextBufferNum) {
    this.service = service;
    this.initBufferNum = initBufferNum;
    this.bufferSize = bufferSize;
    this.nextBufferNum = nextBufferNum;
    this.bufferMap = new ConcurrentHashMap<>();
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
    int bufferId = seqId.getAndIncrement();
    HpnlBuffer buffer = new HpnlBuffer(bufferId, byteBuffer);
    bufferMap.put(bufferId, buffer);
    service.setBuffer(byteBuffer, bufferSize, bufferId);
  }

  private MemoryService service;
  private int initBufferNum;
  private int bufferSize;
  private int nextBufferNum;
  private ConcurrentHashMap<Integer, HpnlBuffer> bufferMap;
  private AtomicInteger seqId;
}
