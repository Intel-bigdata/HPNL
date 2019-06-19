package com.intel.hpnl.api;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class HpnlConfig {

  private String providerName;
  private String fabricFilename;
  private String endpointType;
  private String nic;

  public static final String DEFAULT_VALUE_ENDPOINT_TYPE = "RDM";
  public static final String DEFAULT_PROVIDER_NAME = "sockets";
  public static final String DEFAULT_LIBFABRIC_FILE_NAME = "libfabric.so";

  private static HpnlConfig _instance = new HpnlConfig();

  private static final Logger log = LoggerFactory.getLogger(HpnlConfig.class);

  private HpnlConfig(){
    String path = "/hpnl/hpnl.conf";

    try(InputStream is = HpnlConfig.class.getResourceAsStream(path)){
      Properties properties = new Properties();
      properties.load(is);
      providerName = properties.getProperty("provider_name");
      if (!hasValue(providerName)) {
        providerName = DEFAULT_PROVIDER_NAME;
      }

      fabricFilename = properties.getProperty("libfabric_file_name");
      if (!hasValue(fabricFilename)) {
        fabricFilename = DEFAULT_LIBFABRIC_FILE_NAME;
      }

      endpointType = properties.getProperty("endpoint_type");
      if (!hasValue(endpointType)) {
        endpointType = DEFAULT_VALUE_ENDPOINT_TYPE;
      }

      nic = properties.getProperty("nic_name");
    } catch (IOException e) {
      log.error("failed to read hpnl config from "+path, e);
    }
  }

  private static boolean hasValue(String value){
    return value != null && value.trim().length()>0;
  }

  public static HpnlConfig getInstance(){
    return _instance;
  }

  public HpnlService.EndpointType getEndpointType() {
    return HpnlService.EndpointType.valueOf(endpointType);
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

}
