package com.intel.hpnl.core;

import com.intel.hpnl.api.*;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RdmService extends AbstractService {
  private List<EventTask> tasks;
  private long nativeHandle;
  private int ctxNum;

  private PortGenerator portGenerator = PortGenerator.getInstance();

  private static String LOCAL_IP ;

  private static final Logger log = LoggerFactory.getLogger(RdmService.class);

//  private Thread taskThread;

  static{
    String nic = HpnlConfig.getInstance().getNic();
    try {
      LOCAL_IP = Utils.getIpFromHostname(Utils.getLocalhost(nic));
    }catch (Exception e){
      LOCAL_IP = "<unknown>";
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
  public int connect(String destIp, int destPort, int cqIndex, Handler connectedCallback, Handler recvCallback) {
    String localIp = LOCAL_IP;
    int localPort = getFreePort();
    long tag = Utils.unique(localIp, localPort);
    RdmConnection conn = (RdmConnection) this.conMap.get(this.get_con(destIp, destPort, localIp,
            localPort, tag, -1L, cqIndex, 0, this.nativeHandle));
    conn.setCqIndex(cqIndex);
    conn.setAddrInfo(destIp, destPort, conn.getSrcAddr(), conn.getSrcPort());
    //use connection id as tag
    sendRequest(conn, cqIndex, conn.getConnectionId(), connectedCallback, recvCallback);
//    setConnection(conn);
    return 1;
  }

  private void sendRequest(RdmConnection connection, int cqIndex, long tag,
                           Handler connectedCallback, Handler recvCallback){
    HpnlBuffer buffer = connection.takeSendBuffer();
    ByteBuffer rawBuffer = buffer.getRawBuffer();
    rawBuffer.position(buffer.getMetadataSize());
    rawBuffer.putLong(connection.getConnectionId());
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
    rawBuffer.putInt(cqIndex);
    rawBuffer.putLong(tag);
    if(log.isDebugEnabled()){
      log.debug("connection ({}) with local name length ({})", connection.getConnectionId(), nameLen);
    }

    buffer.insertMetadata(FrameType.REQ.id());
    rawBuffer.flip();
    //wait ack
    connection.setRecvCallback(new ConnectionAckedHandler(connectedCallback, recvCallback));
    connection.sendConnectRequest(buffer, buffer.remaining());
  }

  @Override
  protected void regCon(long key, long connHandle, String destIp,
                        int destPort, String srcIp, int srcPort, long connectId) {
    RdmConnection con = new RdmConnection(connHandle, this.hpnlService, this.server, ctxNum);
    con.setConnectionId(connectId);
    this.conMap.put(connHandle, con);
    if(!server){
      con.setAddrInfo(destIp, destPort, srcIp, srcPort);
      log.info("created client RDM connection {}:{} -> {}:{}", srcIp, srcPort, destIp, destPort);
    }else{
      if(srcPort == -2){//accepted connection
        //dest port will be reset in later
        int port = getFreePort();
        con.setAddrInfo(destIp, destPort, LOCAL_IP, getFreePort());
        log.info("created accepted RDM connection {}:{} -> {}:{}", LOCAL_IP, port, destIp, destPort);
      }else {
        con.setAddrInfo("<unset>", 0, LOCAL_IP, srcPort);
        log.info("created server RDM connection src {}:{}", LOCAL_IP, srcPort);
      }
    }
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
    long start = System.currentTimeMillis();
    while(!conMap.isEmpty()){
      try {
        Thread.sleep(100);
        if(System.currentTimeMillis() - start >= 500){
          break;
        }
      }catch (InterruptedException e){}
    }
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

  protected native long listen(String localhost, int localPort, long nativeHandle);

  protected native long get_con(String destHost, int destPort, String localhost, int localPort, long tag,
                                long destProviderAddr, int cqIndex, int sendCtxId, long nativeHandle);

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
      for(Map.Entry<Long, Connection> entry : RdmService.this.conMap.entrySet()){
        ((RdmConnection)entry.getValue()).doShutdown(true);
      }
      RdmService.this.conMap.clear();
    }
  }

  protected class ConnectionAckedHandler implements Handler{
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
      connection.setRecvCallback(recvHandler);
      connectedHandler.handle(connection, -1, -1);
      return Handler.RESULT_DEFAULT;
    }
  }
}
