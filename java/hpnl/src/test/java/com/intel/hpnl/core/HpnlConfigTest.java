package com.intel.hpnl.core;

import com.intel.hpnl.api.Constants;
import com.intel.hpnl.api.HpnlBufferAllocator;
import com.intel.hpnl.api.HpnlConfig;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.powermock.reflect.Whitebox;

import java.nio.ByteBuffer;

public class HpnlConfigTest {

  private static final String appId = "xyz";

//  @BeforeClass
  public static void setProperty(){
    System.setProperty(Constants.CONFIG_ITEM_APP_ID, appId);
  }

//  @Test
  public void testGetConfigFromSystemProperties()throws Exception{
    HpnlConfig config = Whitebox.invokeConstructor(HpnlConfig.class);
    Assert.assertEquals(appId, config.getAppId());
  }

  @Test
  public void testThreadLocal()throws Exception{
    ThreadLocal<Object> threadLocal = ThreadLocal.withInitial(() -> new Object());
    Object o1 = threadLocal.get();
    System.out.println(o1.hashCode());
    new Thread(()->{
      Object o2 = threadLocal.get();
      System.out.println(o2.hashCode());
    }).start();
    Thread.sleep(3);
    System.out.println(o1.hashCode());
  }
}
