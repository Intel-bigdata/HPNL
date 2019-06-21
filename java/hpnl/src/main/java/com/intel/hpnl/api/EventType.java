package com.intel.hpnl.api;

public class EventType {
  public static final int ACCEPT_EVENT = 1;
  public static final int CONNECTED_EVENT = 2;
  public static final int READ_EVENT = 4;
  public static final int WRITE_EVENT = 8;
  public static final int RECV_EVENT = 16;
  public static final int SEND_EVENT = 32;
  public static final int CLOSE_EVENT = 64;
  public static final int ERROR_EVENT = 128;
  public static final int CONNECT = 256;
  public static final int ACCEPT = 512;
  public static final int SHUTDOWN = 1024;

  private EventType() {
  }
}
