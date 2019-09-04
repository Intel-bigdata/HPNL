package com.intel.hpnl.core;

import com.intel.hpnl.api.HpnlFactory;
import org.junit.Test;

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
