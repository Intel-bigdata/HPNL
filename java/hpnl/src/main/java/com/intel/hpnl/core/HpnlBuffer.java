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

import java.nio.ByteBuffer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class HpnlBuffer {
  
  private static final Logger LOG = LoggerFactory.getLogger(HpnlBuffer.class);

  public HpnlBuffer(int bufferId, ByteBuffer byteBuffer) {
    this.bufferId = bufferId;
    this.byteBuffer = byteBuffer;
  }

  public HpnlBuffer(int bufferId, ByteBuffer byteBuffer, long rkey) {
    this.bufferId = bufferId;
    this.byteBuffer = byteBuffer;
    this.rkey = rkey;
  }

  public HpnlBuffer(int bufferId, ByteBuffer byteBuffer, long rkey, long address) {
    this.bufferId = bufferId;
    this.byteBuffer = byteBuffer;
    this.rkey = rkey;
    this.address = address;
  }

  public class Type {
    public final static byte MSG = 0;
    public final static byte RDM = 1;
  }

  public int getBufferId() {
    return this.bufferId;
  }

  public byte getType() {
    return this.type;
  }

  public long getSeq() {
    return this.seq; 
  }

  public ByteBuffer getRawBuffer() {
    return this.byteBuffer;
  }

  public long getRKey() {
    return this.rkey;
  }

  public long getAddress() {
    return address;
  }

  public ByteBuffer getName() {
    return name.slice(); 
  }

  public int size() {
    return this.byteBuffer.remaining();
  }

  private void putMetadata(int srcSize, byte type, long seq) {
    byteBuffer.rewind();
    int lim = METADATA_SIZE + srcSize;
    byteBuffer.limit(lim);
    byteBuffer.put(Type.MSG);
    byteBuffer.put(type);
    byteBuffer.putLong(seq);
  }

  private void putMetadata(int srcSize, int nameLength, ByteBuffer name, byte type, long seq) {
    byteBuffer.rewind();
    int lim = METADATA_SIZE + srcSize + nameLength + 4;
    byteBuffer.limit(lim);
    byteBuffer.put(Type.RDM);
    byteBuffer.putInt(nameLength);
    byteBuffer.put(name.slice());
    byteBuffer.put(type);
    byteBuffer.putLong(seq);
  }

  public void put(ByteBuffer src, byte type, long seq) {
    putMetadata(src.remaining(), type, seq);
    byteBuffer.put(src.slice());
    byteBuffer.flip();
  }

  public void put(ByteBuffer src, int nameLength, ByteBuffer name, byte type, long seq) {
    try {
      putMetadata(src.remaining(), nameLength, name, type, seq);
    } catch (ArithmeticException e) {
      e.printStackTrace(); 
    }
    byteBuffer.put(src.slice());
    byteBuffer.flip();
  }

  public ByteBuffer get(int blockBufferSize) {
    byteBuffer.position(0); 
    byteBuffer.limit(blockBufferSize);
    if (byteBuffer.get() == Type.MSG) {
      this.type = byteBuffer.get();
      this.seq = byteBuffer.getLong();
      return byteBuffer.slice();
    } else {
      this.nameLength = byteBuffer.getInt();
      byte[] nameTmp = new byte[this.nameLength];
      byteBuffer.get(nameTmp, 0, this.nameLength);
      if (this.name == null) {
        this.name = ByteBuffer.allocateDirect(this.nameLength);
      }
      this.name.put(nameTmp);
      this.name.flip();
      this.type = byteBuffer.get();
      this.seq = byteBuffer.getLong();
      return byteBuffer.slice();
    }
  }

  public int getWritableBytes(){
    return this.byteBuffer.capacity() - METADATA_SIZE;
  }

  private int bufferId;
  private byte type;
  private long seq;
  private int nameLength;
  private ByteBuffer name;
  private ByteBuffer byteBuffer;
  private long rkey;
  private long address;

  private static int METADATA_SIZE = 10;
}
