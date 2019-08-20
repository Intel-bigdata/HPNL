package com.intel.hpnl.core.rdmsglthdiffsize;

import com.intel.hpnl.api.*;
import com.intel.hpnl.core.rdm4.RecvCallback;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.LinkedBlockingQueue;


public class ServerTest {

  private HpnlService service;
  private String hostname;

  public ServerTest(int numThreads, int numBuffer, int bufferSize, String hostname) {
    service = HpnlFactory.getService(numThreads, numBuffer, bufferSize, 50, true);
    this.hostname = hostname;
  }

  public void start()throws Exception {

    service.bind(hostname, 12345, 0, new Handler() {
      @Override
      public int handle(Connection connection, int bufferId, int bufferSize) {
        ByteBuffer rawBuffer = connection.getRecvBuffer(bufferId).getRawBuffer();
        rawBuffer.get();//skip frame type;
        ByteBuffer peerName = connection.getPeerName(rawBuffer.getLong());
        connection.setRecvCallback(new RecvCallback(true, 5, 4096, peerName));
        return 0;
      }
    }, null);
  }

  public static void main(String... args)throws Exception {
    int numThreads = Integer.valueOf(System.getProperty("numThreads", "1"));
    int numBuffer = Integer.valueOf(System.getProperty("numBuffer", "200"));;
    int bufferSize = Integer.valueOf(System.getProperty("bufferSize", "32768"));
    String hostname = System.getProperty("hostname", "10.100.0.35");

    ServerTest test = new ServerTest(numThreads, numBuffer, bufferSize, hostname);
    test.start();
  }
}
