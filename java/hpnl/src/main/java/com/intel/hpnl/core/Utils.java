package com.intel.hpnl.core;

import java.io.IOException;
import java.net.ServerSocket;

public class Utils {
  public Utils() {
  }

  public static int getPort() {
    ServerSocket socket = null;
    boolean var1 = false;

    try {
      socket = new ServerSocket(0);
      socket.setReuseAddress(true);
      int port = socket.getLocalPort();

      try {
        socket.close();
      } catch (IOException var13) {
      }

      int var2 = port;
      return var2;
    } catch (IOException var14) {
    } finally {
      if (socket != null) {
        try {
          socket.close();
        } catch (IOException var12) {
        }
      }

    }

    throw new IllegalStateException("Could not find a free TCP/IP port.");
  }
}
