package com.intel.hpnl.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.nio.ByteBuffer;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

/**
 * A client service for libfabric's event queue
 */
public class EqService {
  private long nativeHandle;
  private long localEq;
  private int workerNum;//number of threads/tasks of CQ service
  private int bufferNbr;//number of buffer per connection
  private int bufferSize;
  public boolean server;
  //TODO: tuning concurrency
  protected Map<Long, Connection> conMap;

  //TODO: tuning concurrency
  private ConcurrentHashMap<Integer, ByteBuffer> rmaBufferMap;

  //global buffer pools
  private MemPool sendBufferPool;
  private MemPool recvBufferPool;

  AtomicInteger rmaBufferId;

  private volatile CountDownLatch connectLatch;

  private EventTask eqTask;

  private Map<Long, Handler> connectedHandlers = new ConcurrentHashMap<>();

  private CqService cqService;

  private AtomicLong nextConnectId = new AtomicLong();

  private static String providerName; //libfabric's provider name
  private static String fabricFilename;//name of libfabric file reference by libhpnl.so

  private static final Logger log = LoggerFactory.getLogger("com.intel.hpnl.core.EqService");

  //load hpnl configuration and dynamic libraries
  static {
    providerName = null;
    fabricFilename = "libfabric.so";
    String path = "/hpnl/hpnl.conf";
    try(InputStream is = EqService.class.getResourceAsStream(path)){
      Properties properties = new Properties();
      properties.load(is);
      providerName = properties.getProperty("provider_name");
      if(providerName != null && providerName.length() == 0){
        providerName = null;
      }
      fabricFilename = properties.getProperty("libfabric_file_name");
      if(fabricFilename != null && fabricFilename.length() == 0){
        fabricFilename = "libfabric.so";
      }
    }catch (IOException e){
      log.error("no hpnl/hpnl.conf found", e);
    }

    try {
      log.info("loading libhpnl.so");
      System.loadLibrary("hpnl");
    }catch (UnsatisfiedLinkError error){
      log.info("failed to load from lib directory. loading from jar instead");
      loadFromJar();
    }
  }

  protected EqService(int workerNum, int bufferNbr, int bufferSize, boolean server) {
    this.workerNum = workerNum;
    this.bufferNbr = bufferNbr;
    this.bufferSize = bufferSize;
    this.server = server;
    this.rmaBufferId = new AtomicInteger(0);

    this.conMap = new ConcurrentHashMap();
    this.rmaBufferMap = new ConcurrentHashMap();
    this.eqTask = new EqTask();
    nextConnectId.set(new Random().nextLong());
  }

  public EqService(int workerNum, int bufferNbr, int bufferSize) {
    this(workerNum, bufferNbr, bufferSize, false);
  }

  /**
   * Initialize native resources and buffers
   * @return
   */
  public EqService init() {
    if (init(workerNum, bufferNbr, server, providerName) == -1)
      return null;
    initBufferPool(bufferNbr, bufferSize, bufferNbr);
    return this; 
  }

  /**
   * try to bind or connect to given <code>ip</code>:<code>port</code> and listen
   * CQ event on <code>cqIndex</code>
   * @param ip
   * @param port
   * @param cqIndex
   * @return
   */
  protected long tryConnect(String ip, String port, int cqIndex){
    long id = sequenceId();
    localEq = internal_connect(ip, port, cqIndex, id, nativeHandle);
    if (localEq == -1) {
      return -1L;
    }
    add_eq_event(localEq, nativeHandle);
    return id;
  }

  /**
   * asynchronously bind or connect to given <code>ip</code>:<code>port</code> with given
   * <code>connectedCallback</code>, and listen CQ event on <code>cqIndex</code>
   * @param ip
   * @param port
   * @param cqIndex
   * @param connectedCallback
   * @return
   */
  public int connect(String ip, String port, int cqIndex, Handler connectedCallback) {
    long seqId = tryConnect(ip, port, cqIndex);
    if(seqId < 0){
      return -1;
    }
    if(connectedCallback != null) {
      Handler prv = connectedHandlers.putIfAbsent(seqId, connectedCallback);
      if(prv != null){
        throw new RuntimeException("non-unique id found, "+seqId);
      }
    }

    return 0;
  }

  /**
   * register newly established connection with connection ID and addresses from JNI
   * @param eq
   * @param con
   * @param dest_addr
   * @param dest_port
   * @param src_addr
   * @param src_port
   * @param connectId
   */
  private void regCon(long eq, long con,
                      String dest_addr, int dest_port, String src_addr, int src_port, long connectId) {
    Connection connection = new Connection(eq, con, this, this.cqService, connectId);
    connection.setAddrInfo(dest_addr, dest_port, src_addr, src_port);
    conMap.put(eq, connection);
  }

  /**
   * should be invoked from JNI when there is a connection SHUTDOWN event.
   * Java method may call it too to make sure connection is removed.
   * @param eq
   */
  public void unregCon(long eq) {
    conMap.remove(eq);
  }

  private static long sequenceId() {
    return Math.abs(UUID.randomUUID().getLeastSignificantBits());
  }

  /**
   * handle EQ callback. invoked from JNI.
   * connectedCallback is invoked here if it's registered.
   * @param eq
   * @param eventType
   * @param blockId
   */
  protected void handleEqCallback(long eq, int eventType, int blockId) {
    Connection connection = conMap.get(eq);
    if (eventType == EventType.CONNECTED_EVENT) {
      long id = connection.getConnectionId();
      Handler connectedHandler =  connectedHandlers.remove(id);
      if(connectedHandler != null){
        connectedHandler.handle(connection, 0, 0);
      }
    }
  }

  public Connection getCon(long eq) {
    return conMap.get(eq);
  }

  public long getNativeHandle() {
    return nativeHandle;
  }

  /**
   * initialize buffers
   * @param initBufferNum
   * @param bufferSize
   * @param nextBufferNum
   */
  private void initBufferPool(int initBufferNum, int bufferSize, int nextBufferNum) {
    this.sendBufferPool = new MemPool(this, initBufferNum, bufferSize, nextBufferNum, MemPool.Type.SEND);
    this.recvBufferPool = new MemPool(this, initBufferNum*2, bufferSize, nextBufferNum*2, MemPool.Type.RECV);
  }

  /**
   * allocate more buffers when more connections are established
   */
  public void reallocBufferPool() {
    this.sendBufferPool.realloc();
    this.recvBufferPool.realloc();
  }

  /**
   * invoked from JNI after buffers are assigned to connection and connection is registered.
   * Java method can then take send buffer from connection.
   * @param eq
   * @param rdmaBufferId
   */
  public void putSendBuffer(long eq, int rdmaBufferId) {
    Connection connection = conMap.get(eq);
    connection.putSendBuffer(sendBufferPool.getBuffer(rdmaBufferId));
  }

  public RdmaBuffer getSendBuffer(int rdmaBufferId) {
    return sendBufferPool.getBuffer(rdmaBufferId); 
  }

  public RdmaBuffer getRecvBuffer(int rdmaBufferId) {
    return recvBufferPool.getBuffer(rdmaBufferId);
  }

  /**
   * register buffer for RDMA
   * @param byteBuffer
   * @param bufferSize
   * @return
   */
  public RdmaBuffer regRmaBuffer(ByteBuffer byteBuffer, int bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    rmaBufferMap.put(bufferId, byteBuffer);
    long rkey = reg_rma_buffer(byteBuffer, bufferSize, bufferId, nativeHandle);
    if (rkey < 0) {
      return null;
    }
    RdmaBuffer buffer = new RdmaBuffer(bufferId, byteBuffer, rkey);
    return buffer;
  }

  /**
   * register RDMA buffer by address
   * @param byteBuffer
   * @param address
   * @param bufferSize
   * @return
   */
  public RdmaBuffer regRmaBufferByAddress(ByteBuffer byteBuffer, long address, long bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    if (byteBuffer != null) {
      rmaBufferMap.put(bufferId, byteBuffer);
    }
    long rkey = reg_rma_buffer_by_address(address, bufferSize, bufferId, nativeHandle);
    if (rkey < 0) {
      return null;
    }
    RdmaBuffer buffer = new RdmaBuffer(bufferId, byteBuffer, rkey);
    return buffer;
  }

  /**
   * un-register RDMA buffer by <code>rdmaBufferId</code>
   * @param rdmaBufferId
   */
  public void unregRmaBuffer(int rdmaBufferId) {
    unreg_rma_buffer(rdmaBufferId, nativeHandle);
  }

  /**
   * allocate and register new RDMA buffer
   * @param bufferSize
   * @return
   */
  public RdmaBuffer getRmaBuffer(int bufferSize) {
    int bufferId = this.rmaBufferId.getAndIncrement();
    // allocate memory from on-heap, off-heap or AEP.
    ByteBuffer byteBuffer = ByteBuffer.allocateDirect(bufferSize);
    long address = get_buffer_address(byteBuffer, nativeHandle);
    rmaBufferMap.put(bufferId, byteBuffer);
    long rkey = reg_rma_buffer(byteBuffer, bufferSize, bufferId, nativeHandle);
    if (rkey < 0) {
      return null;
    }
    RdmaBuffer buffer = new RdmaBuffer(bufferId, byteBuffer, rkey, address);
    return buffer; 
  }

  public void setCqService(CqService cqService){
    this.cqService = cqService;
  }

  public CqService getCqService() {
    return cqService;
  }

  public ByteBuffer getRmaBufferByBufferId(int rmaBufferId) {
    return rmaBufferMap.get(rmaBufferId); 
  }

  public int getWorkerNum() {
    return this.workerNum;
  }

  public EventTask getEventTask(){
    return eqTask;
  }

  /**
   * load libfabric.so and libhpnl.so from jar
   */
  private static void loadFromJar(){
    File tempDir = null;
    try {
      tempDir = Files.createTempDirectory("hpnl").toFile();
      tempDir.deleteOnExit();
      loadByPath("/hpnl/"+fabricFilename, tempDir);
      loadByPath("/hpnl/libhpnl.so", tempDir);
    }catch (IOException e){
      if(tempDir != null){
        tempDir.delete();
      }
      throw new RuntimeException("failed to load libfabric and libhpnl from jar", e);
    }
  }

  private static void loadByPath(String path, File tempDir)throws IOException{
    File tempFile = null;
    String fields[] = path.split("/");
    String name = fields[fields.length-1];
    try(InputStream is = EqService.class.getResourceAsStream(path)){
      tempFile = new File(tempDir, name);
      tempFile.deleteOnExit();
      Files.copy(is, tempFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
      System.load(tempFile.getAbsolutePath());
    }catch (IOException e){
      if(tempFile != null){
        tempFile.delete();
      }
      throw e;
    }
  }

  //native methods
  public native void shutdown(long eq, long nativeHandle);
  private native long internal_connect(String ip, String port, int cqIndex, long connectId, long nativeHandle);
  public native int wait_eq_event(long nativeHandle);
  public native int add_eq_event(long eq, long nativeHandle);
  public native int delete_eq_event(long eq, long nativeHandle);
  public native void set_recv_buffer(ByteBuffer buffer, long size, int rdmaBufferId, long nativeHandle);
  public native void set_send_buffer(ByteBuffer buffer, long size, int rdmaBufferId, long nativeHandle);
  private native long reg_rma_buffer(ByteBuffer buffer, long size, int rdmaBufferId, long nativeHandle);
  private native long reg_rma_buffer_by_address(long address, long size, int rdmaBufferId, long nativeHandle);
  private native void unreg_rma_buffer(int rdmaBufferId, long nativeHandle);
  private native long get_buffer_address(ByteBuffer buffer, long nativeHandle);
  private native int init(int worker_num_, int buffer_num_, boolean is_server_, String prov_name);
  private native void free(long nativeHandle);
  public native void finalize();

  /**
   * stop EQ service by,
   * - stop EQ task and wait its completion
   * - delete EQ event
   * - free JNI resources
   */
  public void stop() {
    eqTask.stop();
    delete_eq_event(localEq, nativeHandle);
    waitToComplete();
    free(nativeHandle);
  }

  private void waitToComplete() {
    try {
      eqTask.waitToComplete();
    } catch (InterruptedException e) {
      log.error("EQ task interrupted when wait its completion", e);
    } finally {
      log.info("EQ task stopped? {}", eqTask.isStopped());
    }
  }

  public long getNewConnectionId() {
    return nextConnectId.incrementAndGet();
  }

  protected class EqTask extends EventTask {

    @Override
    public void waitEvent() {
      if (wait_eq_event(nativeHandle) == -1) {
        log.warn("wait or process EQ event error, ignoring");
      }
    }

    @Override
    protected Logger getLogger(){
      return log;
    }

    @Override
    protected void cleanUp(){
      log.info("close and remove all connections");
      Iterator<Connection> it = conMap.values().iterator();
      while(it.hasNext()){
        it.next().shutdown();
        it.remove();
      }
    }
  }
}
