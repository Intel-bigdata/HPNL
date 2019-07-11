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

package com.intel.hpnl.rma;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.HpnlBuffer;
import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.EqService;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;

public class ServerRecvCallback implements Handler {
  public ServerRecvCallback(EqService eqService) {
    this.eqService = eqService;
    
    this.buf = eqService.getRmaBuffer(4096*1024);
    assert(buf != null);
    for (int i = 0; i < 4096*1024; i++) {
      buf.getRawBuffer().put((byte)i); 
    }
    buf.getRawBuffer().flip();
  }
  public synchronized void handle(Connection con, int bufferId, int blockBufferSize) {
    this.cur_buf = PooledByteBufAllocator.DEFAULT.directBuffer(4096*1024, 4096*1024);
    if (last_buf != null) {
      last_buf.release(); 
    }
    this.last_buf = cur_buf;
    this.rmaBuffer = eqService.regRmaBufferByAddress(this.cur_buf.nioBuffer(0, 4096*1024), this.cur_buf.memoryAddress(), 4096*1024);
    assert(rmaBuffer != null);
    for (int i = 0; i < 4096*1024; i++) {
      rmaBuffer.getRawBuffer().put((byte)i); 
    }
    ByteBuffer byteBufferTmp = ByteBuffer.allocate(16);
    byteBufferTmp.putLong(this.buf.getAddress());
    byteBufferTmp.putLong(this.buf.getRKey());

    byteBufferTmp.flip();
    con.send(byteBufferTmp, (byte)0, 0);
  }
  private EqService eqService;
  private HpnlBuffer rmaBuffer;
  private HpnlBuffer buf;
  private ByteBuf last_buf = null;
  private ByteBuf cur_buf = null;
}
