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
