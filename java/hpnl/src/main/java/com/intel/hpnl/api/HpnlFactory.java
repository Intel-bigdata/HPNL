package com.intel.hpnl.api;

import com.intel.hpnl.api.HpnlService.EndpointType;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.CopyOption;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

import com.intel.hpnl.core.HpnlRdmBuffer;
import com.intel.hpnl.core.RdmHpnlService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class HpnlFactory {

  private static HpnlConfig config = HpnlConfig.getInstance();

  private static boolean loaded;

  private static final Logger log = LoggerFactory.getLogger(HpnlFactory.class);


  static {
    loadLib();
  }

  public static void loadLib(){
    if(loaded){
      return;
    }
    try {
      log.info("loading libhpnl.so");
      System.loadLibrary("hpnl");
    } catch (UnsatisfiedLinkError e) {
      log.info("failed to load from lib directory. loading from jar instead");
      loadFromJar();
    }
    loaded = true;
  }

  private static void loadFromJar() {
    File tempDir = null;

    try {
      tempDir = Files.createTempDirectory("hpnl").toFile();
      tempDir.deleteOnExit();
      loadByPath("/hpnl/" + config.getLibfabricFileName(), tempDir);
      loadByPath("/hpnl/libhpnl.so", tempDir);
    } catch (IOException e) {
      if (tempDir != null) {
        tempDir.delete();
      }

      throw new RuntimeException("failed to load libfabric and libhpnl from jar", e);
    }
  }

  private static void loadByPath(String path, File tempDir) throws IOException {
    File tempFile = null;
    String[] fields = path.split("/");
    String name = fields[fields.length - 1];

    try (InputStream is = HpnlFactory.class.getResourceAsStream(path)){
      tempFile = new File(tempDir, name);
      tempFile.deleteOnExit();
      Files.copy(is, tempFile.toPath(), new CopyOption[]{StandardCopyOption.REPLACE_EXISTING});
      System.load(tempFile.getAbsolutePath());
    } catch (IOException e){
      if (tempFile != null) {
        tempFile.delete();
      }
    }

  }

  public static int getHpnlBufferMetadataSize() {
    EndpointType endpointType = config.getEndpointType();
    switch(endpointType) {
//      case MSG:
//        return HpnlMsgBuffer.METADATA_SIZE;
      case RDM:
        return HpnlRdmBuffer.BASE_METADATA_SIZE;
      default:
        throw new UnsupportedOperationException("unsupported: " + endpointType);
    }
  }

  public static HpnlService getService(int numThreads, int numBuffers, int numRecvBuffers, int bufferSize, boolean server) {
    EndpointType endpointType = config.getEndpointType();
    switch(endpointType) {
//      case MSG:
//        return new MsgHpnlService(numThreads, numBuffers, numRecvBuffers, bufferSize, server);
      case RDM:
        return new RdmHpnlService(numThreads, numBuffers, numRecvBuffers, bufferSize, server);
      default:
        throw new UnsupportedOperationException(endpointType + " is not supported");
    }
  }

}
