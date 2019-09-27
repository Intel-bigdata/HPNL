package com.intel.hpnl.core.rdm5;

import com.intel.hpnl.api.*;

import java.nio.ByteBuffer;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

public class ClientTest {

  private HpnlService service;
  private String hostname;

  private int msgSize;

  private BlockingQueue<Runnable> queue = new LinkedBlockingQueue<>();

  public ClientTest(int numThreads, int numBuffer, int bufferSize, int msgSize, String hostname) {
    service = HpnlFactory.getService(numThreads, numBuffer, bufferSize, 50, false);
    this.hostname = hostname;
    this.msgSize = msgSize;
  }

  public void start()throws Exception{
    EventTask task = service.getCqTask(0);
    Thread th = new Thread(()->{
      System.out.println("starting");
      while(!Thread.currentThread().isInterrupted()) {
        task.run();
      }
      System.out.println("ending");
    });

    th.setUncaughtExceptionHandler((Thread t, Throwable e) -> {
      e.printStackTrace();
    });

    th.start();
    service.connect(hostname, 12345, 0, new Handler() {
      @Override
      public int handle(Connection connection, int bufferId, int bufferSize) {
        try {
          Thread.sleep(1);
        }catch (InterruptedException e){
          e.printStackTrace();
        }
        System.out.println("pingpong");
        //start pingpong
        ByteBuffer byteBufferTmp = ByteBuffer.allocate(msgSize);
        assert(byteBufferTmp != null);
        for (int i = 0; i < msgSize; i++) {
          byteBufferTmp.put((byte)0);
        }
        byteBufferTmp.flip();

        HpnlBuffer buffer = HpnlBufferAllocator.getBufferFromDefault(8192);
        buffer.clear();
        ByteBuffer rawBuffer = buffer.getRawBuffer();
        rawBuffer.position(4113);
//        rawBuffer.put(byteBufferTmp);
        buffer.insertMetadata(FrameType.NORMAL.id());
        rawBuffer.flip();
        connection.sendBuffer(buffer, rawBuffer.remaining());
        return 0;
      }
    }, new RecvCallback(false, 5, 4096, null));

    System.out.println("waiting");
    th.join();
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
