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

}
