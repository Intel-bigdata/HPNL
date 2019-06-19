package com.intel.hpnl.core;

import java.io.IOException;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.SocketException;
import java.util.Enumeration;

public class Utils {

  private Utils() {}

  public static int getPort()throws IOException {
    try (ServerSocket socket = new ServerSocket(0)){
      socket.setReuseAddress(true);
      int port = socket.getLocalPort();
      return port;
    }
  }

  public static String getLocalhost(String nic)throws SocketException {
    Enumeration<NetworkInterface> it = NetworkInterface.getNetworkInterfaces();
    while(it.hasMoreElements()){
      NetworkInterface ni = it.nextElement();
      System.out.println(ni.getName());
      Enumeration<InetAddress> it2 = ni.getInetAddresses();
      while(it2.hasMoreElements()){
        System.out.println(it2.nextElement().getHostAddress());
      }
    }
    return null;
  }

}
