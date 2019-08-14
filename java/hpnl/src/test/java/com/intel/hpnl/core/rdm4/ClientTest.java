package com.intel.hpnl.core.rdm4;

import com.intel.hpnl.api.*;
import com.intel.hpnl.core.RdmConnection;
import com.intel.hpnl.core.RdmHpnlService;
import picocli.CommandLine;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ClientTest implements Runnable {

  private int numThreads = 1;
  private int nbrOfBuffer = 100;
  private int bufferSize = 32768;
  private int ioRatio = 50;

  private String hostname;
  private int port;

  private Connection connection;

  public void run(){
    Object[] ret = startClient();
    try {
      Thread.sleep(5000);
    }catch (Exception e){
      e.printStackTrace();
    }
    ((RdmConnection)ret[1]).shutdown();
    ((RdmHpnlService)ret[0]).stop();
    try {
      Thread.sleep(5000);
    }catch (Exception e){
      e.printStackTrace();
    }
    System.out.println("xyz");
    startClient();
  }

  private Object[] startClient(){
    HpnlService service = HpnlFactory.getService(numThreads, nbrOfBuffer, bufferSize, ioRatio, false);
    service.connect(hostname, port,
            0, new ConnectedHandler());


//    ExecutorService es = Executors.newFixedThreadPool(1);
//    es.submit(hpnlService.getRdmService().getEventTask());
//
//    Object[] ret = new Object[2];
//
//    ret[0] = hpnlService;
//    hpnlService.connect(addr, Integer.valueOf(port), 0, new Handler() {
//      @Override
//      public int handle(Connection connection, int bufferId, int bufferSize) {
//        ret[1] = connection;
//        connection.setRecvCallback(new RecvCallback(false));
//        HpnlBuffer sendBuffer = connection.takeSendBuffer();
//        ByteBuffer rawBuffer = sendBuffer.getRawBuffer();
//        rawBuffer.clear();
//        rawBuffer.position(sendBuffer.getMetadataSize());
//        ByteBuffer localName = connection.getLocalName();
//        localName.rewind();
//        rawBuffer.putInt(localName.remaining());
//        rawBuffer.put(localName);
//        int limit = rawBuffer.position();
//        sendBuffer.insertMetadata((byte)0, -1L, limit);
//        rawBuffer.flip();
//        connection.send(sendBuffer.remaining(), sendBuffer.getBufferId());
//
//        return Handler.RESULT_DEFAULT;
//      }
//    });
    return null;
  }

  public static void main(String... args) {
    HpnlFactory.loadLib();
    CommandLine.run(new ClientTest(), args);
  }

  private class ConnectedHandler implements Handler{

    @Override
    public int handle(Connection connection, int bufferId, int bufferSize) {
      ClientTest.this.connection = connection;
      return 0;
    }
  }
}
