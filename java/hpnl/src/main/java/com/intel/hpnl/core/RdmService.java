package com.intel.hpnl.core;

import com.intel.hpnl.api.*;

import java.nio.ByteBuffer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RdmService extends AbstractService {
  private EventTask task;
  private long nativeHandle;
  protected String localIp;
  protected int localPort;

  private PortGenerator portGenerator = PortGenerator.getInstance();

  private static String LOCAL_IP ;

  private static final Logger log = LoggerFactory.getLogger(RdmService.class);

  private Thread taskThread;

  static{
    String nic = HpnlConfig.getInstance().getNic();
    try {
      LOCAL_IP = Utils.getLocalhost(nic);
    }catch (Exception e){
      LOCAL_IP = null;
      log.error("failed to get local IP of NIC: "+nic, e);
    }
  }

  public RdmService(int workNum, int bufferNum, int bufferSize, int ioRatio) {
    this(workNum, bufferNum, bufferSize, ioRatio, false);
  }

  protected RdmService(int workNum, int bufferNum, int bufferSize, int ioRatio, boolean server) {
    super(workNum, bufferNum, bufferSize, ioRatio, server);
  }

  public RdmService init() {
    this.init(this.bufferNum, this.server, HpnlConfig.getInstance().getLibfabricProviderName());
    this.initBufferPool(this.bufferNum, this.bufferSize, this.bufferNum);
    this.task = new RdmService.RdmTask(ioRatio);
    taskThread = new Thread(task);
    Runtime.getRuntime().addShutdownHook(new Thread(() -> this.shutdown()));
    return this;
  }

  public void start(){
    taskThread.start();
  }

  @Override
  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    synchronized (this) {
      RdmConnection conn = (RdmConnection) this.conMap.get(this.get_con(ip, port, this.nativeHandle));
      conn.setAddrInfo(ip, Integer.valueOf(port), conn.getSrcAddr(), conn.getSrcPort());
      connectedCallback.handle(conn, -1, -1);
    }
    return 1;
  }

  @Override
  protected void regCon(long key, long connHandle, String dest_addr, int dest_port, String src_addr, int src_port, long connectId) {
    RdmConnection con = new RdmConnection(connHandle, this.hpnlService, this.server);
    if(!server){
      localIp = LOCAL_IP;
      localPort = getFreePort();
      con.setAddrInfo(con.getDestAddr(), con.getDestPort(), localIp, localPort);
    }else{
      con.setAddrInfo("<unset>", 0, localIp, localPort);
    }
    con.setConnectionId(localIp, localPort);
    this.conMap.put(connHandle, con);
  }

  @Override
  public void unregCon(long connHandle) {}

  @Override
  public void removeConnection(long nativeConnectionId, long connHandle, boolean proactive) {
    remove_connection(nativeConnectionId, nativeHandle);
//    RdmConnection connection = (RdmConnection)this.conMap.remove(connHandle);
//    if(!connection.isServer()){
//      portGenerator.reclaimPort(connection.getSrcPort());
//    }
  }

  @Override
  public EventTask getEventTask() {
    return this.task;
  }

  @Override
  public void stop() {
    if(!this.task.isStopped()) {
      synchronized (this) {
        if(taskThread != null) {
          taskThread.interrupt();
          taskThread = null;
        }
        if (!this.task.isStopped()) {
          this.task.stop();
//          this.waitToComplete();
        }
      }
    }
  }

  private void shutdown(){
    log.info(this+" shutting down");
    stop();
    conMap.forEach((k, v) -> {
      RdmConnection connection = (RdmConnection)v;
      if(!connection.isServer()) {
        portGenerator.reclaimPort(v.getSrcPort());
      }
    });
    conMap.clear();
    this.free(this.nativeHandle);
    log.info(this+" shut down");
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

  @Override
  protected void setSendBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_send_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  @Override
  protected void setRecvBuffer(ByteBuffer buffer, long size, int bufferId) {
    this.set_recv_buffer(buffer, size, bufferId, this.nativeHandle);
  }

  @Override
  protected HpnlBuffer newHpnlBuffer(int bufferId, ByteBuffer byteBuffer){
    return new HpnlRdmBuffer(bufferId, byteBuffer);
  }

  private native int init(int bufferNum, boolean server, String providerName);

  protected native long listen(String host, String port, long nativeHandle);

  private native long get_con(String host, String port, long nativeHandle);

  private native int wait_event(long nativeHandle);

  public native void set_recv_buffer(ByteBuffer buffer, long size, int id, long nativeHandle);

  public native void set_send_buffer(ByteBuffer buffer, long size, int id, long nativeHandle);

  public native void remove_connection(long id, long nativeHandle);

  private native void free(long nativeHandle);

  public int getFreePort(){
    return portGenerator.getFreePort();
  }

  public void reclaimPort(int port){
    portGenerator.reclaimPort(port);
  }

  protected class RdmTask extends EventTask {
    protected RdmTask(int ioRatio) {
      super(ioRatio);
    }

    @Override
    public int waitEvent() {
      int ret = RdmService.this.wait_event(RdmService.this.nativeHandle) ;
      if(ret == -1) {
        RdmService.log.warn("wait or process event error, ignoring");
      }
      return ret;
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
