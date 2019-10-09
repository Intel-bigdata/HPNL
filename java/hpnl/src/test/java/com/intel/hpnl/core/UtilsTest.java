package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlFactory;
import org.junit.Test;

import java.nio.ByteBuffer;

public class UtilsTest {

  @Test
  public void testGetLocalhostByNic()throws Exception{
    System.out.println(Utils.getLocalhost("eno1"));
  }

  @Test
  public void testGetDefaultLocalhost()throws Exception{
    System.out.println(Utils.getLocalhost(null));
  }

  @Test
  public void testGetIpByHostname()throws Exception{
    try {
//      System.out.println(Utils.getIpFromHostname("sr555"));
    }catch (Exception e){

    }
  }

  @Test
  public void testGetPid()throws Exception{
    HpnlFactory.loadLib();
    int pid = Utils.getPid();
    System.out.println(pid);
  }

  @Test
  public void testOps(){
    Inner inner = new Inner();
    Outer outer = new Outer();
    inner.putInt();
    outer.putInt();

    long start = System.nanoTime();
    for(int i=0; i<100000; i++){
      inner.putInt();
    }

    System.out.println(System.nanoTime() - start);
     start = System.nanoTime();
    for(int i=0; i<100000; i++){
      outer.putInt();
    }
    System.out.println(System.nanoTime() - start);

  }

  @Test
  public void testBufferOps() throws Exception{
    ByteBuffer in = ByteBuffer.allocateDirect(100);
    int limit = in.limit();
    in.position(limit - 4);//read head length
    int headLen = in.getInt();
    in.position(0);
    in.position(3);
    in.limit(67);

    long start = System.nanoTime();
    for(int i=0; i<100000; i++){
      limit = in.limit();
      in.position(limit - 4);//read head length
      headLen = in.getInt();
      in.position(0);
      in.position(3);
      in.limit(67);
    }
    System.out.println(System.nanoTime() - start);
  }

  @Test
  public void test1()throws Exception{
    int BUFFER_ID_RANGES = 100000000;
    int MIN_DEFAULT_BUFFER_ID = -BUFFER_ID_RANGES;

    int currentBufferIdLimit = MIN_DEFAULT_BUFFER_ID;
    int count = 0;
    while(currentBufferIdLimit < 0){
      currentBufferIdLimit -= BUFFER_ID_RANGES;
      count++;
    }
    System.out.println(currentBufferIdLimit);
    System.out.println(count);
  }

  static class Inner{
    public void putInt(){}
  }
  static class Outer{
    Inner inner = new Inner();
    public void putInt(){
      inner.putInt();
    }
  }


}
