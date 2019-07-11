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

import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;

public class Server {
  public static void main(String args[]) {
    final int BUFFER_SIZE = 65536;
    final int BUFFER_NUM = 128;

    EqService eqService = new EqService(1, BUFFER_NUM, true).init();
    CqService cqService = new CqService(eqService).init();
    assert(eqService != null);
    assert(cqService != null);
    ServerRecvCallback recvCallback = new ServerRecvCallback(eqService);
    eqService.setRecvCallback(recvCallback);

    eqService.initBufferPool(BUFFER_NUM, BUFFER_SIZE, BUFFER_NUM);

    cqService.start();
    eqService.listen("172.168.2.106", "123456");

    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
