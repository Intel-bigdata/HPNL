package com.intel.hpnl.core;

import com.intel.hpnl.api.Constants;
import com.intel.hpnl.api.HpnlConfig;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.powermock.reflect.Whitebox;

public class HpnlConfigTest {

  private static final String appId = "xyz";

  @BeforeClass
  public static void setProperty(){
    System.setProperty(Constants.CONFIG_ITEM_APP_ID, appId);
  }

  @Test
  public void testGetConfigFromSystemProperties()throws Exception{
    HpnlConfig config = Whitebox.invokeConstructor(HpnlConfig.class);
    Assert.assertEquals(appId, config.getAppId());
  }
}
