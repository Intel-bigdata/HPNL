package com.intel.hpnl.pingpong;

import picocli.CommandLine;
import picocli.CommandLine.Option;
import picocli.CommandLine.Command;

import com.intel.hpnl.service.Server;

@Command(mixinStandardHelpOptions = true, version = "auto help demo - picocli 3.0")
public class ServerTest implements Runnable {

  @Option(names = {"-a", "--address"}, required = false, description = "server address")
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
  int workNbr = 3;
  
  @Option(names = {"-i", "--interval"}, required = false, description = "statistics interval time")
  int interval = 5;

  @Option(names = {"-f", "--affinity"}, required = false, split = ",", description = "HPNL thread affinity")
  int[] affinities = null;

  public void run() {
    Server server = new Server(workNbr, bufferNbr);
    server.setAffinities(affinities);

    RecvCallback recvCallback = new RecvCallback(true, interval, msgSize);
    server.setRecvCallback(recvCallback);

    server.initBufferPool(bufferNbr, bufferSize, bufferNbr);
    server.start();

    server.listen(addr, port);

    server.join();
  }

  public static void main(String... args) {
    CommandLine.run(new ServerTest(), args);
  }
}
