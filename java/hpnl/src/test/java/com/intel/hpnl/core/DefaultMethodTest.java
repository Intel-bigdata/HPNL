package com.intel.hpnl.core;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlBuffer;

public class DefaultMethodTest {

    static class HandlerImp implements Handler{

        @Override
        public int handle(Connection connection, int bufferId, int bufferSize) {
            return Handler.RESULT_DEFAULT;
        }

        @Override
        public int handle(Connection connection, HpnlBuffer hpnlBuffer){
            return Handler.RESULT_DEFAULT;
        }
    }

    public static void main(String args[]){
        HandlerImp h = new HandlerImp();

        h.handle(null, -1, -1);
        h.handle(null, null);



        long start = System.nanoTime();
        for(int i=0; i<100000; i++){
            h.handle(null, null);
        }
        System.out.println(System.nanoTime() - start);

        start = System.nanoTime();
        for(int i=0; i<100000; i++){
            h.handle(null, -1, -1);
        }
        System.out.println(System.nanoTime() - start);
    }
}
