package com.intel.hpnl.core.rdm;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlFactory;
import com.intel.hpnl.core.RdmServerService;
import picocli.CommandLine;
import picocli.CommandLine.Option;
import picocli.CommandLine.Command;

import com.intel.hpnl.core.RdmService;

import java.util.concurrent.Executor;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

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
    RdmService service = new RdmServerService(1, bufferNbr, bufferSize, 50).init();
    assert(service != null);

    ExecutorService es = Executors.newFixedThreadPool(1);
    es.submit(service.getEventTask());

    service.connect(addr, port, 0, new Handler() {
      @Override
      public int handle(Connection connection, int bufferId, int bufferSize) {
        connection.setRecvCallback(new RecvCallback(true));
        return Handler.RESULT_DEFAULT;
      }
    });

  }

  public static void main(String... args) {
    HpnlFactory.loadLib();
    CommandLine.run(new ServerTest(), args);
  }
}
