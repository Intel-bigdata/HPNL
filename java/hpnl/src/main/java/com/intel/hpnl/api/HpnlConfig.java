package com.intel.hpnl.api;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class HpnlConfig {

  private String providerName;
  private String fabricFilename;
  private HpnlService.EndpointType endpointType;
  private String nic;
  private String appId;
  private int portBatchSize;

  private int bufferNumSmall;
  private int bufferNumLarge;
  private int bufferNumMax;

  private int bufferDynNumArena;
  private int bufferDynNumTiny;
  private int bufferDynNumSmall;
  private int bufferDynNumNormal;

  private int bufferSmallCap;
  private int bufferLargeCap;

  private boolean autoAckConnection;

  private int ctxNum;
  private int srvCtxNum;

  private int readBatchSize;

  private Properties properties;

  public static final String DEFAULT_ENDPOINT_TYPE = "RDM";
  public static final String DEFAULT_APP_ID = "default_hpnl";
  public static final String DEFAULT_PORT_BATCH_SIZE = "50";
  public static final String DEFAULT_PROVIDER_NAME = "sockets";
  public static final String DEFAULT_LIBFABRIC_FILE_NAME = "libfabric.so";

  public static final String DEFAULT_CONTEXT_NUM = "1024";
  public static final String DEFAULT_CONTEXT_NUM_SERVER = "2048";

  public static final String DEFAULT_READ_BATCH_SIZE = "10";

  public static final String DEFAULT_BUFFER_NUM_SMALL = "4096";
  public static final String DEFAULT_BUFFER_NUM_LARGE = "2048";
  public static final String DEFAULT_BUFFER_NUM_MAX = "256"; //buffer with max size which is equal to send buffer size

  public static final String DEFAULT_BUFFER_DYN_NUM_ARENA = "0"; // default is processors*2
  public static final String DEFAULT_BUFFER_DYN_NUM_TINY = "512";
  public static final String DEFAULT_BUFFER_DYN_NUM_SMALL = "256";
  public static final String DEFAULT_BUFFER_DYN_NUM_NORMAL = "64";

  public static final String DEFAULT_BUFFER_CAPACITY_SMALL = "4096";
  public static final String DEFAULT_BUFFER_CAPACITY_LARGE = "16384";

  public static final String DEFAULT_AUTO_ACK_CONNECTION = "false";

  private static HpnlConfig _instance = new HpnlConfig();

  private static final Logger log = LoggerFactory.getLogger(HpnlConfig.class);

  private HpnlConfig(){
    String path = "/hpnl/hpnl.conf";

    try(InputStream is = HpnlConfig.class.getResourceAsStream(path)){
      properties = new Properties();
      properties.load(is);
      providerName = getValue(Constants.CONFIG_ITEM_PROVIDER_NAME, DEFAULT_PROVIDER_NAME);

      fabricFilename = getValue(Constants.CONFIG_ITEM_LIBFABRIC_FILE_NAME, DEFAULT_LIBFABRIC_FILE_NAME);

      String endPointValue = getValue(Constants.CONFIG_ITEM_END_POINT_TYPE, DEFAULT_ENDPOINT_TYPE);
      endpointType = HpnlService.EndpointType.valueOf(endPointValue);

      nic = getValue(Constants.CONFIG_ITEM_NIC_NAME, null);

      appId = getValue(Constants.CONFIG_ITEM_APP_ID, DEFAULT_APP_ID);

      String tmp = getValue(Constants.CONFIG_ITEM_PORT_BATCH_SIZE, DEFAULT_PORT_BATCH_SIZE);
      portBatchSize = Integer.valueOf(tmp);

      tmp = getValue(Constants.CFG_BUFFER_NUM_SMALL, DEFAULT_BUFFER_NUM_SMALL);
      bufferNumSmall = Integer.valueOf(tmp);
      tmp = getValue(Constants.CFG_BUFFER_NUM_LARGE, DEFAULT_BUFFER_NUM_LARGE);
      bufferNumLarge = Integer.valueOf(tmp);
      tmp = getValue(Constants.CFG_BUFFER_NUM_MAX, DEFAULT_BUFFER_NUM_MAX);
      bufferNumMax = Integer.valueOf(tmp);

      tmp = getValue(Constants.CFG_BUFFER_DYN_NUM_ARENA, DEFAULT_BUFFER_DYN_NUM_ARENA);
      bufferDynNumArena = Integer.valueOf(tmp);
      tmp = getValue(Constants.CFG_BUFFER_DYN_NUM_TINY, DEFAULT_BUFFER_DYN_NUM_TINY);
      bufferDynNumTiny = Integer.valueOf(tmp);
      tmp = getValue(Constants.CFG_BUFFER_DYN_NUM_SMALL, DEFAULT_BUFFER_DYN_NUM_SMALL);
      bufferDynNumSmall = Integer.valueOf(tmp);
      tmp = getValue(Constants.CFG_BUFFER_DYN_NUM_NORMAL, DEFAULT_BUFFER_DYN_NUM_NORMAL);
      bufferDynNumNormal = Integer.valueOf(tmp);

      tmp = getValue(Constants.CFG_BUFFER_CAPACITY_SMALL, DEFAULT_BUFFER_CAPACITY_SMALL);
      bufferSmallCap = Integer.valueOf(tmp);
      tmp = getValue(Constants.CFG_BUFFER_CAPACITY_LARGE, DEFAULT_BUFFER_CAPACITY_LARGE);
      bufferLargeCap = Integer.valueOf(tmp);


      tmp = getValue(Constants.CFG_AUTO_ACK_CONNECTION, DEFAULT_AUTO_ACK_CONNECTION);
      autoAckConnection = Boolean.valueOf(tmp);

      tmp = getValue(Constants.CFG_CONTEXT_NUM, DEFAULT_CONTEXT_NUM);
      ctxNum = Integer.valueOf(tmp);
      tmp = getValue(Constants.CFG_CONTEXT_NUM_SERVER, DEFAULT_CONTEXT_NUM_SERVER);
      srvCtxNum = Integer.valueOf(tmp);

      tmp = getValue(Constants.CFG_READ_BATCH_SIZE, DEFAULT_READ_BATCH_SIZE);
      readBatchSize = Integer.valueOf(tmp);
    } catch (IOException e) {
      log.error("failed to read hpnl config from "+path, e);
    }
  }

  public String getValue(String key, String defaultValue){
    String value = System.getProperty(key, properties.getProperty(key));
    if (!hasValue(value)) {
      value = defaultValue;
    }
    return value;
  }

  public String getValueByRole(String role, String key, String defaultValue){
    String prefix = "hpnl.";
    if(key.startsWith(prefix)){
      key = new StringBuffer(prefix).append(role).append('.').append(key.substring(prefix.length())).toString();
    }else{
      key = new StringBuffer(role).append('.').append(key).toString();
    }
    return getValue(key, defaultValue);
  }

  private static boolean hasValue(String value){
    return value != null && value.trim().length()>0;
  }

  public static HpnlConfig getInstance(){
    return _instance;
  }

  public HpnlService.EndpointType getEndpointType() {
    return endpointType;
  }

  public String getLibfabricProviderName() {
    return providerName;
  }

  public String getLibfabricFileName() {
    return fabricFilename;
  }

  public String getNic(){
    return nic;
  }

  public String getAppId() {
    return appId;
  }

  public int getPortBatchSize() {
    return portBatchSize;
  }

  public int getCtxNum(String role) {
    if(role == null) {
      return ctxNum;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_CONTEXT_NUM, DEFAULT_CONTEXT_NUM));
  }

  public int getSrvCtxNum(String role) {
    if(role == null) {
      return srvCtxNum;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_CONTEXT_NUM_SERVER, DEFAULT_CONTEXT_NUM_SERVER));
  }

  public int getBufferNumSmall(String role) {
    if(role == null) {
      return bufferNumSmall;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_BUFFER_NUM_SMALL, DEFAULT_BUFFER_NUM_SMALL));
  }

  public int getBufferNumLarge(String role) {
    if(role == null) {
      return bufferNumLarge;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_BUFFER_NUM_LARGE, DEFAULT_BUFFER_NUM_LARGE));
  }

  public int getBufferNumMax(String role) {
    if(role == null) {
      return bufferNumMax;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_BUFFER_NUM_MAX, DEFAULT_BUFFER_NUM_MAX));
  }

  public boolean isAutoAckConnection (String role){
    if(role == null) {
      return autoAckConnection;
    }
    return Boolean.valueOf(getValueByRole(role, Constants.CFG_AUTO_ACK_CONNECTION, DEFAULT_AUTO_ACK_CONNECTION));
  }

  public int getReadBatchSize(String role) {
    if(role == null) {
      return readBatchSize;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_READ_BATCH_SIZE, DEFAULT_READ_BATCH_SIZE));
  }

  public int getBufferSmallCap(String role) {
    if(role == null) {
      return bufferSmallCap;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_BUFFER_CAPACITY_SMALL, DEFAULT_BUFFER_CAPACITY_SMALL));
  }

  public int getBufferLargeCap(String role) {
    if(role == null) {
      return bufferLargeCap;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_BUFFER_CAPACITY_LARGE, DEFAULT_BUFFER_CAPACITY_LARGE));
  }

  public int getBufferDynNumArena(String role) {
    if(role == null) {
      return bufferDynNumArena;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_BUFFER_DYN_NUM_ARENA, DEFAULT_BUFFER_DYN_NUM_ARENA));
  }

  public int getBufferDynNumTiny(String role) {
    if(role == null) {
      return bufferDynNumTiny;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_BUFFER_DYN_NUM_TINY, DEFAULT_BUFFER_DYN_NUM_TINY));
  }

  public int getBufferDynNumSmall(String role) {
    if(role == null) {
      return bufferDynNumSmall;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_BUFFER_DYN_NUM_SMALL, DEFAULT_BUFFER_DYN_NUM_SMALL));
  }

  public int getBufferDynNumNormal(String role) {
    if(role == null) {
      return bufferDynNumNormal;
    }
    return Integer.valueOf(getValueByRole(role, Constants.CFG_BUFFER_DYN_NUM_NORMAL, DEFAULT_BUFFER_DYN_NUM_NORMAL));
  }
}
