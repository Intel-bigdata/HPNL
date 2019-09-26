package com.intel.hpnl.core;

import com.intel.hpnl.api.*;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RdmService extends AbstractService {
  private List<EventTask> tasks;
  private long nativeHandle;
  protected String localIp;
  protected int localPort;
  private int ctxNum;

  private PortGenerator portGenerator = PortGenerator.getInstance();

  private static String LOCAL_IP ;

  private static final Logger log = LoggerFactory.getLogger(RdmService.class);

//  private Thread taskThread;

  static{
    String nic = HpnlConfig.getInstance().getNic();
    try {
      LOCAL_IP = Utils.getLocalhost(nic);
    }catch (Exception e){
      LOCAL_IP = null;
      log.error("failed to get local IP of NIC: "+nic, e);
    }
  }

  public RdmService(int workNum, int bufferNum, int numRecvBuffers, int bufferSize) {
    this(workNum, bufferNum, numRecvBuffers, bufferSize, false);
  }

  protected RdmService(int workNum, int bufferNum, int numRecvBuffers, int bufferSize,
                       boolean server) {
    super(workNum, bufferNum, numRecvBuffers, bufferSize, server);
    tasks = new ArrayList<>();
    for(int i=0; i<workNum; i++){
      tasks.add(new RdmTask(i));
    }
    tasks = Collections.unmodifiableList(tasks);
  }

  public RdmService init() {
    return init(HpnlConfig.getInstance().getCtxNum());
  }

  protected RdmService init(int ctxNum) {
    this.ctxNum = ctxNum;

    this.init(this.bufferNum, recvBufferNum, ctxNum, workerNum,
            HpnlConfig.getInstance().getReadBatchSize(), this.server,
            HpnlConfig.getInstance().getLibfabricProviderName());
    this.initBufferPool(this.bufferNum, recvBufferNum, this.bufferSize);
//    taskThread = new Thread(task);
    Runtime.getRuntime().addShutdownHook(new Thread(() -> this.shutdown()));
    return this;
  }

  @Override
  public int connect(String ip, String port, int cqIndex, Handler connectedCallback, Handler recvCallback) {
    RdmConnection conn = (RdmConnection) this.conMap.get(this.get_con(ip, port,
            -1L, cqIndex, 0, this.nativeHandle));
    conn.setAddrInfo(ip, Integer.valueOf(port), conn.getSrcAddr(), conn.getSrcPort());

    sendRequest(conn, connectedCallback, recvCallback);
    return 1;
  }

  private void sendRequest(RdmConnection connection, Handler connectedCallback, Handler recvCallback){
    HpnlBuffer buffer = connection.takeSendBuffer();
    ByteBuffer rawBuffer = buffer.getRawBuffer();
    rawBuffer.position(buffer.getMetadataSize());
    String ipPort = new StringBuffer(connection.getSrcAddr()).append(":").append(connection.getSrcPort())
            .toString();
    byte[] bytes = ipPort.getBytes();
    rawBuffer.putInt(bytes.length);
    rawBuffer.put(bytes);

    ByteBuffer localName = connection.getLocalName();
    localName.rewind();
    int nameLen = localName.remaining();
    rawBuffer.putInt(nameLen);
    rawBuffer.put(localName);
    if(log.isDebugEnabled()){
      log.debug("connection ({}) with local name length ({})", connection.getConnectionId(), nameLen);
    }

    int limit = rawBuffer.position();
    buffer.insertMetadata(FrameType.REQ.id(), -1L, limit);
    rawBuffer.flip();
    //wait ack
    connection.setRecvCallback(new ConnectionAckedHandler(connectedCallback, recvCallback));
    connection.sendConnectRequest(buffer, buffer.remaining());
  }

  @Override
  protected void regCon(long key, long connHandle, String dest_addr,
                        int dest_port, String src_addr, int src_port, long connectId) {
    RdmConnection con = new RdmConnection(connHandle, this.hpnlService, this.server, ctxNum);
    if(!server){
      localIp = LOCAL_IP;
      localPort = getFreePort();
      con.setAddrInfo(con.getDestAddr(), con.getDestPort(), localIp, localPort);
    }else{
      con.setAddrInfo("<unset>", 0, localIp, localPort);
      if(connectId == -2L){//accepted connection
        localPort = getFreePort();
      }
    }
    con.setConnectionId(localIp, localPort);
    this.conMap.put(connHandle, con);
    log.info("registered connection {}:{}", localIp, localPort);
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
  public EventTask getEventTask(int cqIndex) {
    if(cqIndex >= workerNum){
      throw new IllegalArgumentException(String.format(
              "cq index %d cannot exceed number of worker threads %d", cqIndex, workerNum));
    }
    return tasks.get(cqIndex);
  }

  public List<EventTask> getCqTasks(){
    return tasks;
  }

  @Override
  public void stop() {
//    if(!this.task.isStopped()) {
//        synchronized (this) {
//            if (taskThread != null) {
//                taskThread.interrupt();
//                taskThread = null;
//            }
//            if (!this.task.isStopped()) {
//                this.task.stop();
//          this.waitToComplete();
//            }
//        }
//    }
  }

  private void shutdown(){
    log.info(this+" shutting down");
    stop();
//    conMap.forEach((k, v) -> {
//      RdmConnection connection = (RdmConnection)v;
//      if(!connection.isServer()) {
//        portGenerator.reclaimPort(v.getSrcPort());
//      }
//    });
    conMap.clear();
    this.free(this.nativeHandle);
    log.info(this+" shut down");
  }

//  private void waitToComplete() {
//    try {
//      this.task.waitToComplete();
//    } catch (InterruptedException var5) {
//      log.error("EQ task interrupted when wait its completion", var5);
//    } finally {
//      log.info("EQ task stopped? {}", this.task.isStopped());
//    }
//
//  }

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
  protected HpnlBuffer newHpnlBuffer(int bufferId, ByteBuffer byteBuffer, HpnlBuffer.BufferType type){
    return new HpnlRdmBuffer(bufferId, byteBuffer, type);
  }

  private native int init(int bufferNum, int recvBufferNum, int ctxNum, int endpointNum, int readBatchSize,
                          boolean server, String providerName);

  protected native long listen(String host, String port, long nativeHandle);

  protected native long get_con(String host, String port, long destProviderAddr, int cqIndex, int sendCtxId, long nativeHandle);

  private native int wait_event(int cqIndex, long nativeHandle);

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
    private int cqIndex;
    protected RdmTask(int cqIndex) {
      this.cqIndex = cqIndex;
    }

    @Override
    public int waitEvent() {
      int ret = RdmService.this.wait_event(cqIndex, RdmService.this.nativeHandle) ;
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

  protected static class ConnectionAckedHandler implements Handler{
    private Handler connectedHandler;
    private Handler recvHandler;

    public ConnectionAckedHandler(Handler connectedHandler, Handler recvHandler){
      this.connectedHandler = connectedHandler;
      this.recvHandler = recvHandler;
    }

    @Override
    public int handle(Connection connection, int bufferId, int bufferSize) {
      HpnlBuffer buffer = connection.getRecvBuffer(bufferId);
      buffer.parse(bufferSize);
      return handle(connection, buffer);
    }

    @Override
    public int handle(Connection connection, HpnlBuffer buffer){
      if(buffer.getFrameType() != FrameType.ACK.id()){
        throw new RuntimeException(
                String.format("expect message type %d, actual %d", FrameType.ACK.id(), buffer.getFrameType()));
      }
      int sendCtxId = buffer.getInt();
      connection.adjustSendTarget(sendCtxId);
      if(log.isDebugEnabled()){
        byte[] bytes = new byte[buffer.getInt()];
        buffer.get(bytes);
        log.debug("got ack with content, "+new String(bytes));
      }
      connectedHandler.handle(connection, -1, -1);
      connection.setRecvCallback(recvHandler);
      return Handler.RESULT_DEFAULT;
    }
  }
}
