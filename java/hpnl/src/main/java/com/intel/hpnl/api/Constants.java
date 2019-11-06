package com.intel.hpnl.api;

public class Constants {

  private Constants(){};

  public static final String CONFIG_ITEM_PROVIDER_NAME = "provider_name";
  public static final String CONFIG_ITEM_LIBFABRIC_FILE_NAME = "libfabric_file_name";
  public static final String CONFIG_ITEM_END_POINT_TYPE = "endpoint_type";
  public static final String CONFIG_ITEM_NIC_NAME = "nic_name";
  public static final String CONFIG_ITEM_APP_ID = "app_id";
  public static final String CONFIG_ITEM_PORT_BATCH_SIZE = "port_batch_size";

  public static final String CFG_BUFFER_NUM_SMALL = "hpnl.buffer.num.small";
  public static final String CFG_BUFFER_NUM_LARGE = "hpnl.buffer.num.large";
  public static final String CFG_BUFFER_NUM_MAX = "hpnl.buffer.num.max";

  public static final String CFG_BUFFER_DYN_NUM_ARENA = "hpnl.buffer.dynamic.num.arena";
  public static final String CFG_BUFFER_DYN_NUM_TINY = "hpnl.buffer.dynamic.num.tiny";
  public static final String CFG_BUFFER_DYN_NUM_SMALL = "hpnl.buffer.dynamic.num.small";
  public static final String CFG_BUFFER_DYN_NUM_NORMAL = "hpnl.buffer.dynamic.num.normal";

  public static final String CFG_BUFFER_CAPACITY_SMALL = "hpnl.buffer.capacity.small";
  public static final String CFG_BUFFER_CAPACITY_LARGE = "hpnl.buffer.capacity.large";

  public static final String CFG_AUTO_ACK_CONNECTION = "hpnl.auto.ack.connection";

  public static final String CFG_CONTEXT_NUM = "hpnl.context.num";
  public static final String CFG_CONTEXT_NUM_SERVER = "hpnl.context.num.server";

  public static final String CFG_READ_BATCH_SIZE = "hpnl.read.batch.size";

}
