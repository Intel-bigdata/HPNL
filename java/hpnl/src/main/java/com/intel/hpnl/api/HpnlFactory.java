package com.intel.hpnl.api;

import com.intel.hpnl.api.HpnlService.EndpointType;
import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.MsgHpnlService;
import com.intel.hpnl.core.RdmHpnlService;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.CopyOption;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.Properties;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class HpnlFactory {
  public static final String DEFAULT_VALUE_ENDPOINT_TYPE = "RDM";
  public static final String DEFAULT_PROVIDER_NAME = "sockets";
  public static final String DEFAULT_LIBFABRIC_FILE_NAME = "libfabric.so";
  private static String providerName = null;
  private static String fabricFilename = null;
  private static String endpointType;
  private static final Logger log = LoggerFactory.getLogger(HpnlFactory.class);

  public HpnlFactory() {
  }

  private static void loadFromJar() {
    File tempDir = null;

    try {
      tempDir = Files.createTempDirectory("hpnl").toFile();
      tempDir.deleteOnExit();
      loadByPath("/hpnl/" + getLibfabricFileName(), tempDir);
      loadByPath("/hpnl/libhpnl.so", tempDir);
    } catch (IOException var2) {
      if (tempDir != null) {
        tempDir.delete();
      }

      throw new RuntimeException("failed to load libfabric and libhpnl from jar", var2);
    }
  }

  private static void loadByPath(String path, File tempDir) throws IOException {
    File tempFile = null;
    String[] fields = path.split("/");
    String name = fields[fields.length - 1];

    try {
      InputStream is = EqService.class.getResourceAsStream(path);
      Throwable var6 = null;

      try {
        tempFile = new File(tempDir, name);
        tempFile.deleteOnExit();
        Files.copy(is, tempFile.toPath(), new CopyOption[]{StandardCopyOption.REPLACE_EXISTING});
        System.load(tempFile.getAbsolutePath());
      } catch (Throwable var16) {
        var6 = var16;
        throw var16;
      } finally {
        if (is != null) {
          if (var6 != null) {
            try {
              is.close();
            } catch (Throwable var15) {
              var6.addSuppressed(var15);
            }
          } else {
            is.close();
          }
        }

      }

    } catch (IOException var18) {
      if (tempFile != null) {
        tempFile.delete();
      }

      throw var18;
    }
  }

  public static EndpointType getEndpointType() {
    if (endpointType == null) {
      endpointType = "RDM";
    }

    return EndpointType.valueOf(endpointType);
  }

  public static String getLibfabricProviderName() {
    if (providerName == null) {
      providerName = "sockets";
    }

    return providerName;
  }

  public static String getLibfabricFileName() {
    if (fabricFilename == null) {
      fabricFilename = "libfabric.so";
    }

    return fabricFilename;
  }

  public static int getHpnlBufferMetadataSize() {
    EndpointType endpointType = getEndpointType();
    switch(endpointType) {
      case MSG:
        return 9;
      case RDM:
        return 17;
      default:
        throw new UnsupportedOperationException("unsupported: " + endpointType);
    }
  }

  public static HpnlService getService(int numThreads, int numBuffers, int bufferSize, boolean server) {
    EndpointType endpointType = getEndpointType();
    switch(endpointType) {
      case MSG:
        return new MsgHpnlService(numThreads, numBuffers, bufferSize, server);
      case RDM:
        return new RdmHpnlService(numThreads, numBuffers, bufferSize, server);
      default:
        throw new UnsupportedOperationException(endpointType + " is not supported");
    }
  }

  static {
    String path = "/hpnl/hpnl.conf";

    try {
      InputStream is = EqService.class.getResourceAsStream(path);
      Throwable var2 = null;

      try {
        Properties properties = new Properties();
        properties.load(is);
        providerName = properties.getProperty("provider_name");
        if (providerName != null && providerName.length() == 0) {
          providerName = "sockets";
        }

        fabricFilename = properties.getProperty("libfabric_file_name");
        if (fabricFilename != null && fabricFilename.length() == 0) {
          fabricFilename = "libfabric.so";
        }

        endpointType = properties.getProperty("endpoint_type");
        if (endpointType != null && endpointType.length() == 0) {
          endpointType = "RDM";
        }
      } catch (Throwable var14) {
        var2 = var14;
        throw var14;
      } finally {
        if (is != null) {
          if (var2 != null) {
            try {
              is.close();
            } catch (Throwable var13) {
              var2.addSuppressed(var13);
            }
          } else {
            is.close();
          }
        }

      }
    } catch (IOException var16) {
      log.error("no hpnl/hpnl.conf found", var16);
    }

    try {
      log.info("loading libhpnl.so");
      System.loadLibrary("hpnl");
    } catch (UnsatisfiedLinkError var12) {
      log.info("failed to load from lib directory. loading from jar instead");
      loadFromJar();
    }

  }
}
