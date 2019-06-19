package com.intel.hpnl.core;

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
}
