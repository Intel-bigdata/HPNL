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

import java.net.ServerSocket;
import java.io.IOException;

public class Utils {
  public static int getPort() {
    ServerSocket socket = null;
    int port;
    try {
      socket = new ServerSocket(0);
      socket.setReuseAddress(true);
      port = socket.getLocalPort();
      try {
        socket.close();
      } catch (IOException e) {
        e.printStackTrace();
      }
      return port;
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (socket != null) {
        try {
          socket.close();
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
    }
    throw new IllegalStateException("Could not find a free TCP/IP port.");
  }

  public static void setAffinity(long affinity) {
    if (affinity > 0L) {
      set_affinity(affinity);
    }
  }

  private static native void set_affinity(long affinity);
}
