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

package com.intel.hpnl.buffer;

import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.EqService;

import java.nio.ByteBuffer;

public class Client {
  public static void main(String args[]) {
    ByteBuffer byteBufferTmp = ByteBuffer.allocate(4096);
    for (int i = 0; i < 4096; i++) {
      byteBufferTmp.put((byte)0);
    }
    byteBufferTmp.flip();

    try {
      String addr = args.length >=1 ? args[0] : "localhost";
      int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
      int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;
      EqService eqService = new EqService(1, bufferNbr, false).init();
      CqService cqService = new CqService(eqService).init();
      assert(eqService != null);
      assert(cqService != null);

      ReadCallback readCallback = new ReadCallback();
      ShutdownCallback shutdownCallback = new ShutdownCallback();
      eqService.setRecvCallback(readCallback);
      eqService.setSendCallback(null);
      eqService.setShutdownCallback(shutdownCallback);

      eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

      cqService.start();
      eqService.connect(addr, "123456", 0);

      System.out.println("connected, start to pingpong.");

      //cqService.shutdown();
      cqService.join();
      eqService.shutdown();
      eqService.join();
    } catch (NumberFormatException e) {
      e.printStackTrace(); 
    }
  }
}
