package com.intel.hpnl.buffer;

import com.intel.hpnl.core.CqService;
import com.intel.hpnl.core.EqService;

public class Server {
  public static void main(String args[]) {
    String addr = args.length >=1 ? args[0] : "localhost";
    int bufferSize = args.length >=2 ? Integer.valueOf(args[1]) : 65536;
    int bufferNbr = args.length >=3 ? Integer.valueOf(args[2]) : 32;
    int workNbr = args.length >=4 ? Integer.valueOf(args[3]) : 3;

    EqService eqService = new EqService(workNbr, bufferNbr, true).init();
    CqService cqService = new CqService(eqService, eqService.getNativeHandle()).init();
    
    eqService.initBufferPool(bufferNbr, bufferSize, bufferNbr);

    eqService.listen(addr, "123456");
    cqService.start();
    
    cqService.join();
    eqService.shutdown();
    eqService.join();
  }
}
