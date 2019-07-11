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

public class EventType {
  public final static int ACCEPT_EVENT = 1;
  public final static int CONNECTED_EVENT = 2;
  public final static int READ_EVENT = 4;
  public final static int WRITE_EVENT = 8;
  public final static int RECV_EVENT = 16;
  public final static int SEND_EVENT = 32;
  public final static int CLOSE_EVENT = 64;
  public final static int ERROR_EVENT = 128;
  public final static int CONNECT = 256;
  public final static int ACCEPT = 512;
  public final static int SHUTDOWN = 1024;

}
