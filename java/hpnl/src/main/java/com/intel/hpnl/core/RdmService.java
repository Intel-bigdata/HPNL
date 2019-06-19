package com.intel.hpnl.core;

import com.intel.hpnl.api.*;

import java.nio.ByteBuffer;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RdmService extends AbstractService {
  private EventTask task;
  private long nativeHandle;
  protected String localIp;
  protected int localPort;
  protected String peerIp;
  protected int peerPort;

  private static String LOCAL_IP ;

  private static Map<Integer, Object> portRegister = new ConcurrentHashMap<>();
  private static AtomicInteger portGenerator = new AtomicInteger(1);

  private static Object dummyValue = new Object();

  private static final Logger log = LoggerFactory.getLogger(RdmService.class);

  static{
    String nic = HpnlConfig.getInstance().getNic();
    try {
      LOCAL_IP = Utils.getLocalhost(nic);
    }catch (Exception e){
      LOCAL_IP = null;
      log.error("failed to get local IP of NIC: "+nic, e);
    }
  }

  public RdmService(int workNum, int bufferNum, int bufferSize) {
    this(workNum, bufferNum, bufferSize, false);
  }

  protected RdmService(int workNum, int bufferNum, int bufferSize, boolean server) {
    super(workNum, bufferNum, bufferSize, server);
  }

  public RdmService init() {
    this.init(this.bufferNum, this.server, HpnlConfig.getInstance().getLibfabricProviderName());
    this.initBufferPool(this.bufferNum, this.bufferSize, this.bufferNum);
    this.task = new RdmService.RdmTask();
    return this;
  }

  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    peerIp = ip;
    peerPort = Integer.valueOf(port);
    Connection conn = this.conMap.get(this.get_con(ip, port, this.nativeHandle));
    connectedCallback.handle(conn, -1, -1);
    return 1;
  }

  protected void regCon(long key, long connHandle, String dest_addr, int dest_port, String src_addr, int src_port, long connectId) {
    RdmConnection con = new RdmConnection(connHandle, this.hpnlService, this.server);
    if(!server){
      localIp = LOCAL_IP;
      localPort = getFreePort();
    }else{
      peerIp = "<unset>";
    }
    con.setAddrInfo(peerIp, peerPort, localIp, localPort);
    con.setConnectionId(localIp, localPort);
    this.conMap.put(connHandle, con);
  }

  public void unregCon(long connHandle) {
    RdmConnection connection = (RdmConnection)this.conMap.remove(connHandle);
    if(connection != null && !connection.isServer()){
      portRegister.remove(connection.getSrcPort());
    }
  }

  public void removeConnection(long nativeConnectionId, long connHandle, boolean proactive) {
    remove_connection(nativeConnectionId, nativeHandle);
    this.conMap.remove(connHandle);
  }

  public EventTask getEventTask() {
    return this.task;
  }

  public void stop() {
    if(!stopped) {
      synchronized (this) {
        if (!stopped) {
          this.task.stop();
          this.waitToComplete();
          conMap.forEach((k, v) -> {
            RdmConnection connection = (RdmConnection)v;
            if(!connection.isServer()) {
              portRegister.remove(Integer.valueOf(v.getSrcPort()));
            }
          });
          conMap.clear();
          this.free(this.nativeHandle);
          stopped = true;
        }
      }
    }
  }

  private void waitToComplete() {
    try {
      this.task.waitToComplete();
    } catch (InterruptedException var5) {
      log.error("EQ task interrupted when wait its completion", var5);
    } finally {
      log.info("EQ task stopped? {}", this.task.isStopped());
    }

  }

  public long getNativeHandle() {
    return this.nativeHandle;
  }

  protected void setSendBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_send_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  protected void setRecvBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_recv_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  private native int init(int var1, boolean var2, String nativeHandle);

  protected native long listen(String var1, String var2, long nativeHandle);

  private native long get_con(String var1, String var2, long nativeHandle);

  private native int wait_event(long nativeHandle);

  public native void set_recv_buffer(ByteBuffer var1, long var2, int var4, long nativeHandle);

  public native void set_send_buffer(ByteBuffer var1, long var2, int var4, long nativeHandle);

  public native void remove_connection(long id, long nativeHandle);

  private native void free(long nativeHandle);

  public static int getFreePort(){
    int port = getNextValidPort();
    while(portRegister.containsKey(port)){
      port = getNextValidPort();
    }
    portRegister.put(Integer.valueOf(port), dummyValue);
    return port;
  }

  private static int getNextValidPort(){
    int value = portGenerator.getAndIncrement();
    if(value > 65535 || value < 1){
      synchronized (portGenerator){
        value = portGenerator.getAndIncrement();
        if(value > 65535 || value < 1){
          portGenerator.set(1);
          return portGenerator.getAndIncrement();
        }
        return value;
      }
    }
    return value;
  }

  public static void removePortFromRegister(int port){
    portRegister.remove(Integer.valueOf(port));
  }

  protected class RdmTask extends EventTask {
    protected RdmTask() {
    }

    public void waitEvent() {
      if (RdmService.this.wait_event(RdmService.this.nativeHandle) == -1) {
        RdmService.log.warn("wait or process event error, ignoring");
      }

    }

    protected Logger getLogger() {
      return RdmService.log;
    }

    protected void cleanUp() {
      RdmService.log.info("close and remove all connections");
      RdmService.this.conMap.clear();
    }
  }
}
