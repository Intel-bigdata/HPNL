package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlFactory;

import java.io.IOException;
import java.net.*;
import java.util.Enumeration;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Utils {

  public static final Pattern IP_PTN = Pattern.compile("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");

  static{
    HpnlFactory.loadLib();
  }

  private Utils() {}

  public static int getPort()throws IOException {
    try (ServerSocket socket = new ServerSocket(0)){
      socket.setReuseAddress(true);
      int port = socket.getLocalPort();
      return port;
    }
  }

  public static boolean isIp(String host){
    return IP_PTN.matcher(host).matches();
  }

  public static String getLocalhost(String nic)throws SocketException, UnknownHostException {
    if(nic == null || nic.trim().length() == 0){
      return InetAddress.getLocalHost().getHostAddress();
    }
    Enumeration<NetworkInterface> it = NetworkInterface.getNetworkInterfaces();
    while(it.hasMoreElements()){
      NetworkInterface ni = it.nextElement();
      if(nic.equalsIgnoreCase(ni.getName())) {
        Enumeration<InetAddress> it2 = ni.getInetAddresses();
        String ip = null;
        while (it2.hasMoreElements()) {
          InetAddress address = it2.nextElement();
          Matcher matcher = IP_PTN.matcher(address.getHostAddress());
          if(matcher.matches()){
            ip = address.getHostAddress();
            if(!(ip.equals("127.0.0.1") || ip.equals("127.0.1.1"))){
              return ip;
            }
          }
        }
        return ip;
      }
    }
    return null;
  }

  public static String getIpFromHostname(String hostname) throws UnknownHostException {
    InetAddress address = InetAddress.getByName(hostname);
    return address.getHostAddress();
  }

  public static void setAffinitiy(long affinitiy){
    if(affinitiy > 0L){
      set_affinity(affinitiy);
    }
  }

  private static native void set_affinity(long affinity);

  public static int getPid(){
    return get_pid();
  }

  private static native int get_pid();

}
