package com.intel.hpnl.core.rdm4;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlFactory;
import com.intel.hpnl.core.RdmServerService;
import com.intel.hpnl.core.RdmService;
import picocli.CommandLine;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class ServerTest {

  public void run() {
//    RdmService service = new RdmServerService(1, bufferNbr, bufferSize, 50).init();
//    assert(service != null);
//
//    ExecutorService es = Executors.newFixedThreadPool(1);
//    es.submit(service.getEventTask());
//
//    service.connect(addr, port, 0, new Handler() {
//      @Override
//      public int handle(Connection connection, int bufferId, int bufferSize) {
//        connection.setRecvCallback(new RecvCallback(true));
//        return Handler.RESULT_DEFAULT;
//      }
//    });

  }

  public static void main(String... args) {

  }
}
