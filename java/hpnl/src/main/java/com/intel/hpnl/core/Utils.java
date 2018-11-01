package com.intel.hpnl.core;

import java.net.ServerSocket;
import java.io.IOException;

public class Utils {
  public static int getPort() {
    ServerSocket socket = null;
    int port = 0;
    try {
      socket = new ServerSocket(0);
      socket.setReuseAddress(true);
      port = socket.getLocalPort();
      try {
        socket.close();
      } catch (IOException e) {
      }
      return port;
    } catch (IOException e) {
    } finally {
      if (socket != null) {
        try {
          socket.close();
        } catch (IOException e) {
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
