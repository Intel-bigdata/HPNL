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

package com.intel.hpnl.rdm;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.RdmHandler;
import com.intel.hpnl.core.HpnlBuffer;
import com.intel.hpnl.core.RdmConnection;

public class RecvCallback implements RdmHandler {
  public RecvCallback(boolean is_server, int interval, int msgSize) {
    this.is_server = is_server;
    this.interval = interval;
    this.msgSize = msgSize;
  }
  public void handle(RdmConnection con, int bufferId, int blockBufferSize) {
    if (!is_server) {
      count++;
      if (count == 1) {
        startTime = System.currentTimeMillis();
        endTime = startTime;
      } else {
        endTime = System.currentTimeMillis();
        if ((total_time = endTime-startTime) >= interval*1000) {
          latency = total_time*1000/(float)count;
          throughput = count*msgSize/1024/1024/(total_time/1000);
          System.out.println(msgSize + " bytes message, latency " + latency + " us");
          System.out.println(msgSize + " bytes message, throughput " + throughput + " MB/s");
          System.out.println("************************************************************");
          count = 0;
        }
      }
    }
    HpnlBuffer recvBuffer = con.getRecvBuffer(bufferId);
    assert(recvBuffer != null);
    ByteBuffer recvByteBuffer = recvBuffer.get(blockBufferSize);
    con.sendTo(recvByteBuffer, (byte)0, 10, recvBuffer.getName());
  }
  private long count = 0;
  private long startTime;
  private long endTime;
  private float total_time = 0;
  private float latency = 0;
  private float throughput = 0;

  private boolean is_server = false;
  private int interval;
  private int msgSize;
}
