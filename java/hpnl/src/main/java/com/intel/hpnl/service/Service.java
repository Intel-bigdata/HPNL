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

package com.intel.hpnl.service;

import java.nio.ByteBuffer;
import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.HpnlBuffer;
import com.intel.hpnl.core.Handler;

public abstract class Service {
  public Service(int workNbr, int bufferNbr, boolean isServer) {
    this.eqService = new EqService(workNbr, bufferNbr, isServer);
    assert(this.eqService != null);
    this.eqService.init();
    this.cqService = new CqService(this.eqService);
    assert(this.cqService != null);
    this.cqService.init();
  }

  public void initBufferPool(int bufferNbr, int bufferSize, int nextBufferNbr) {
    this.eqService.initBufferPool(bufferNbr, bufferSize, nextBufferNbr);
  }

  public void setAffinities(int[] affinities) {
    this.cqService.setAffinities(affinities);
  }

  public void start() {
    this.cqService.start();
  }

  public void shutdown() {
    this.cqService.shutdown();
    this.cqService.join();
    this.eqService.shutdown();
    this.eqService.join();
  }

  public void join() {
    this.cqService.join();
    this.eqService.join();
  }

  public void setConnectedCallback(Handler callback) {
    this.eqService.setConnectedCallback(callback);
  }

  public void setRecvCallback(Handler callback) {
    this.eqService.setRecvCallback(callback);
  }

  public void setSendCallback(Handler callback) {
    this.eqService.setSendCallback(callback);
  }

  public void setReadCallback(Handler callback) {
    this.eqService.setReadCallback(callback);
  }

  public void setShutdownCallback(Handler callback) {
    this.eqService.setShutdownCallback(callback);
  }

  public HpnlBuffer regRmaBuffer(ByteBuffer byteBuffer, int bufferSize) {
    return this.eqService.regRmaBuffer(byteBuffer, bufferSize);
  }

  public HpnlBuffer regRmaBufferByAddress(ByteBuffer byteBuffer, long address, long bufferSize) {
    return this.eqService.regRmaBufferByAddress(byteBuffer, address, bufferSize);
  }

  public void unregRmaBuffer(int bufferId) {
    this.eqService.unregRmaBuffer(bufferId);
  }

  public HpnlBuffer getRmaBuffer(int bufferSize) {
    return this.eqService.getRmaBuffer(bufferSize);
  }

  public ByteBuffer getRmaBufferByBufferId(int rmaBufferId) {
    return this.eqService.getRmaBufferByBufferId(rmaBufferId);
  }

  protected EqService eqService;
  protected CqService cqService;
}
