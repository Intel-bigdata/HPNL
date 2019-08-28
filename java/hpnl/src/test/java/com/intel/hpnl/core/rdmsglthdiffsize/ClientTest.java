package com.intel.hpnl.core.rdmsglthdiffsize;

import com.intel.hpnl.api.*;
import com.intel.hpnl.core.rdm4.RecvCallback;

import java.nio.ByteBuffer;

public class ClientTest {

  private HpnlService service;
  private String hostname;

  private int msgSize;

  public ClientTest(int numThreads, int numBuffer, int bufferSize, int msgSize, String hostname) {
    service = HpnlFactory.getService(numThreads, numBuffer, bufferSize, 50, false);
    this.hostname = hostname;
    this.msgSize = msgSize;
  }

  public void start()throws Exception{

    service.connect(hostname, 12345, 0, new Handler() {
      @Override
      public int handle(Connection connection, int bufferId, int bufferSize) {
        try {
          Thread.sleep(1);
        }catch (InterruptedException e){
          e.printStackTrace();
        }
        //start pingpong
        ByteBuffer byteBufferTmp = ByteBuffer.allocate(msgSize);
        assert(byteBufferTmp != null);
        for (int i = 0; i < msgSize; i++) {
          byteBufferTmp.put((byte)0);
        }
        byteBufferTmp.flip();

        HpnlBuffer buffer = connection.takeSendBuffer();
        buffer.clear();
        ByteBuffer rawBuffer = buffer.getRawBuffer();
        rawBuffer.position(buffer.getMetadataSize());
        rawBuffer.put(byteBufferTmp);
        int limit = rawBuffer.position();
        buffer.insertMetadata(FrameType.NORMAL.id(), 0L, limit);
        rawBuffer.flip();
        connection.sendBuffer(buffer, buffer.remaining());
        return 0;
      }
    }, new RecvCallback(false, 5, 4096, null));

  }

  public static void main(String... args) throws Exception{
    int numThreads = Integer.valueOf(System.getProperty("numThreads", "1"));
    int numBuffer = Integer.valueOf(System.getProperty("numBuffer", "200"));;
    int bufferSize = Integer.valueOf(System.getProperty("bufferSize", "32768"));
    int msgSize = Integer.valueOf(System.getProperty("msgSize", "4096"));
    String hostname = System.getProperty("hostname", "10.100.0.35");

    ClientTest test = new ClientTest(numThreads, numBuffer, bufferSize, msgSize, hostname);
    test.start();
  }
}
