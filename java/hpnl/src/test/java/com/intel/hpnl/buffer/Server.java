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

public class Server {
  public static void main(String args[]) {
    try {
      String addr = args.length >=1 ? args[0] : "localhost";
      int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
      int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;
      int workNbr = args.length >=4 ? Integer.valueOf(args[3]) : 3;
      EqService eqService = new EqService(workNbr, bufferNbr, true).init();
      CqService cqService = new CqService(eqService).init();
      assert(eqService != null);
      assert(cqService != null);
      
      eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

      eqService.listen(addr, "123456");
      cqService.start();
      
      cqService.join();
      eqService.shutdown();
      eqService.join();
    } catch (NumberFormatException e) {
      e.printStackTrace(); 
    }
  }
}
