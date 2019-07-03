package com.intel.hpnl.core.rdm2;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlBuffer;
import com.intel.hpnl.api.HpnlFactory;
import com.intel.hpnl.core.RdmConnection;
import com.intel.hpnl.core.RdmHpnlService;
import picocli.CommandLine;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;

import java.nio.ByteBuffer;
import java.util.Random;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Command(mixinStandardHelpOptions = true, version = "auto help demo - picocli 3.0")
public class ClientTest implements Runnable {

  @Option(names = {"-a", "--address"}, required = true, description = "server address")
  String addr = "localhost";

  @Option(names = {"-p", "--port"}, required = false, description = "server port")
  String port = "123456";

  @Option(names = {"-s", "--buffer_size"}, required = false, description = "buffer size")
  int bufferSize = 65536;

  @Option(names = {"-n", "--buffer_number"}, required = false, description = "buffer number")
  int bufferNbr = 2000;

  @Option(names = {"-m", "--message_size"}, required = false, description = "pingpong message size")
  int msgSize = 4096;

  @Option(names = {"-w", "--worker_number"}, required = false, description = "worker numbers")
  int workNbr = 1;

  @Option(names = {"-i", "--interval"}, required = false, description = "statistics interval time")
  int interval = 5;

  @Option(names = {"-f", "--affinity"}, required = false, split = ",",  description = "HPNL thread affinity")
  int[] affinities = null;

  public void run(){
    for(int i=0; i<32; i++){
      new Thread(() -> startClient()).start();
      System.out.println("again");
    }
  }

  private Object[] startClient(){
    RdmHpnlService hpnlService = new RdmHpnlService(1, bufferNbr, bufferSize, 50, false);
    assert(hpnlService != null);

    ExecutorService es = Executors.newFixedThreadPool(1);
    es.submit(hpnlService.getRdmService().getEventTask());

    Object[] ret = new Object[2];

    ret[0] = hpnlService;
    hpnlService.connect(addr, Integer.valueOf(port), 0, new Handler() {
      @Override
      public int handle(Connection connection, int bufferId, int bufferSize) {
        ret[1] = connection;
        connection.setRecvCallback(new RecvCallback(false));
        boolean b = true;
        for(int j=0; j<bufferNbr; j++) {
          HpnlBuffer sendBuffer = connection.takeSendBuffer();
          ByteBuffer rawBuffer = sendBuffer.getRawBuffer();
          rawBuffer.clear();
          rawBuffer.position(sendBuffer.getMetadataSize());

          int posBefore = rawBuffer.position();
          rawBuffer.position(posBefore + 4);

//        int len = new Random().nextInt(4000);
          int len = 0;
          if(b) {
            len = 2414;
            for (int i = 0; i < len; i++) {
              rawBuffer.put((byte) i);
            }
          }else{
            len = rawBuffer.remaining();
            for (int i = 0; i < len; i++) {
              rawBuffer.put((byte) i);
            }
          }
          b = !b;
          int pos = rawBuffer.position();
          rawBuffer.position(posBefore);
          rawBuffer.putInt(len);
          rawBuffer.position(pos);
          int limit = rawBuffer.position();
          sendBuffer.insertMetadata((byte) 0, -1L, limit);
          rawBuffer.flip();
          connection.send(sendBuffer.remaining(), sendBuffer.getBufferId());
        }

        return Handler.RESULT_DEFAULT;
      }
    });



    ((Connection)ret[1]).shutdown();
    hpnlService.stop();
    return ret;
  }

  public static void main(String... args) {
    HpnlFactory.loadLib();
    CommandLine.run(new ClientTest(), args);
  }
}
