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

import java.util.concurrent.atomic.AtomicBoolean;

public class CqThread extends Thread {
  public CqThread(CqService cqService, int index, long affinity) {
    this.cqService = cqService;
    this.index = index; 
    this.affinity = affinity;
    running.set(true);
    this.setDaemon(true);
  }

  public void run() {
    if (this.affinity != -1)
      Utils.setAffinity(this.affinity);
    while (running.get()) {
      if (this.cqService.wait_event(index) == -1) {
        shutdown();
      }
    }
    this.cqService.free();
  }

  public void shutdown() {
    running.set(false); 
  }

  private CqService cqService;
  private int index;
  private long affinity;
  private final AtomicBoolean running = new AtomicBoolean(false);
}
