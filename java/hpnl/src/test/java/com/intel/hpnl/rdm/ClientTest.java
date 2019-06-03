package com.intel.hpnl.rdm;

import java.nio.ByteBuffer;

import picocli.CommandLine;
import picocli.CommandLine.Option;
import picocli.CommandLine.Command;

import com.intel.hpnl.core.RdmService;
import com.intel.hpnl.core.RdmConnection;

@Command(mixinStandardHelpOptions = true, version = "auto help demo - picocli 3.0")
public class ClientTest implements Runnable {

  @Option(names = {"-a", "--address"}, required = true, description = "server address")
  String addr = "localhost";

  @Option(names = {"-p", "--port"}, required = false, description = "server port")
  String port = "123456";

  @Option(names = {"-s", "--buffer_size"}, required = false, description = "buffer size")
  int bufferSize = 65536;

  @Option(names = {"-n", "--buffer_number"}, required = false, description = "buffer number")
  int bufferNbr = 32;

  @Option(names = {"-m", "--message_size"}, required = false, description = "pingpong message size")
  int msgSize = 4096;

  @Option(names = {"-w", "--worker_number"}, required = false, description = "worker numbers")
  int workNbr = 1;

  @Option(names = {"-i", "--interval"}, required = false, description = "statistics interval time")
  int interval = 5;

  @Option(names = {"-f", "--affinity"}, required = false, split = ",",  description = "HPNL thread affinity")
  int[] affinities = null;

  public void run() {
    ByteBuffer byteBufferTmp = ByteBuffer.allocate(msgSize);
    for (int i = 0; i < msgSize; i++) {
      byteBufferTmp.put((byte)0);
    }
    byteBufferTmp.flip();

    RdmService service = new RdmService(bufferNbr, false).init();
    
    RecvCallback recvCallback = new RecvCallback(false, interval, msgSize);
    service.setRecvCallback(recvCallback);

    service.initRecvBufferPool(bufferNbr, bufferSize, bufferNbr);
    service.initSendBufferPool(bufferNbr, bufferSize, bufferNbr);

    RdmConnection con = null;
    for (int i = 0; i < 10; i++) {
      con = service.get_con(addr, port);
      con.send(byteBufferTmp, (byte)0, 10);
    }

    service.join();
  }

  public static void main(String... args) {
    CommandLine.run(new ClientTest(), args);
  }
}
