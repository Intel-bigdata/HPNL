package com.intel.hpnl.core;

import com.intel.hpnl.api.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

public class RdmServerService extends RdmService {
  private int cqIndex = 0;
  protected Connection connection;
  private Map<Long, Connection> connectionMap = new HashMap<>();
  private boolean autoAckConnection;

  private static final Logger log = LoggerFactory.getLogger(RdmServerService.class);

  public RdmServerService(int workNum, int bufferNum, int numRecvBuffers, int bufferSize, String role) {
    super(workNum, bufferNum, numRecvBuffers, bufferSize, true, role);
    if(workNum < 2){
        throw new IllegalArgumentException("work number should be at least 2 for server");
    }
    autoAckConnection = HpnlConfig.getInstance().isAutoAckConnection(role);
  }

  protected void setConnection(Connection connection){
    if(this.connection != null){
        throw new IllegalStateException("connection is set already");
    }
    this.connection = connection;
  }

    /**
    * primary connection for service.
    * @return
    */
  @Override
  public Connection getConnection(){
    if(connection == null){
        throw new IllegalStateException("connect is not established yet");
    }
    return connection;
  }

  @Override
  public RdmService init() {
    return init(HpnlConfig.getInstance().getSrvCtxNum(role));
  }

  @Override
  public int connect(String ip, int port, int cqIndex, Handler connectedCallback, Handler recvCallback) {
    RdmConnection conn = (RdmConnection) this.getConnection(this.listen(ip, port, this.getNativeHandle()));
    conn.setCqIndex(cqIndex);
    setConnection(conn);
    conn.setRecvCallback(new RdmServerReceiveHandler(connectedCallback, recvCallback));
    return 1;
  }

  @Override
  public void ackConnected(Connection connection){
    String addr = connection.getDestAddr() + ":" + connection.getDestPort();
    int dataSize = 60;
    HpnlBuffer buffer = connection.takeSendBuffer();
    if(buffer == null) {
        buffer = getBuffer(dataSize + AbstractHpnlBuffer.BASE_METADATA_SIZE);
        ByteBuffer rawBuffer = buffer.getRawBuffer();
        rawBuffer.position(buffer.getMetadataSize());
        rawBuffer.putInt(connection.getCqIndex());
        rawBuffer.putInt(addr.getBytes().length);
        rawBuffer.put(addr.getBytes());
        buffer.insertMetadata(FrameType.ACK.id());
        rawBuffer.flip();
        connection.sendBuffer(buffer, rawBuffer.remaining());
    }else {
        ByteBuffer byteBuffer = ByteBuffer.allocate(dataSize + AbstractHpnlBuffer.BASE_METADATA_SIZE);
        byteBuffer.putInt(connection.getCqIndex());
        byteBuffer.putInt(addr.getBytes().length);
        byteBuffer.put(addr.getBytes());
        byteBuffer.flip();
        buffer.putData(byteBuffer, FrameType.ACK.id());
        connection.sendBuffer(buffer, buffer.remaining());
    }

    if (log.isDebugEnabled()) {
        log.debug("acknowledging connection from " + addr);
    }
  }

  private class RdmServerReceiveHandler implements Handler {
    private Handler connectedCallback;
    private Handler recvCallback;

    public RdmServerReceiveHandler(Handler connectedCallback, Handler recvCallback){
      this.connectedCallback = connectedCallback;
      this.recvCallback = recvCallback;
    }

    @Override
    public int handle(Connection connection, int bufferId, int bufferSize) {
//      log.info("bufferId: {}, {}", bufferId, bufferSize);
      HpnlBuffer buffer = connection.getRecvBuffer(bufferId);
      buffer.parse(bufferSize);
      return handle(connection, buffer);
    }

    @Override
    public int handle(Connection connection, HpnlBuffer hpnlBuffer) {
      FrameType frameType = FrameType.toFrameType(hpnlBuffer.getFrameType());
      if(frameType == FrameType.REQ) {
          long peerConnectId = hpnlBuffer.getLong();
          if(connectionMap.containsKey(peerConnectId)){
              throw new IllegalStateException("child connection has been created already," +
                      " peer connection id is "+peerConnectId);
          }
          Object[] address = getPeerAddress(hpnlBuffer);
          long provAddr = getPeerProviderAddr(hpnlBuffer);
          int cqIndex = RdmServerService.this.nextCqIndex();
          int sendCtxId = hpnlBuffer.getInt();
          long tag = hpnlBuffer.getLong();
          Connection childConnection = RdmServerService.this.createChildConnection(provAddr, address,
                  cqIndex, sendCtxId, tag, recvCallback);
          connectionMap.put(peerConnectId, childConnection);
          connectedCallback.handle(childConnection, hpnlBuffer);
          if(autoAckConnection){
              ackConnected(childConnection);
          }
          return Handler.RESULT_DEFAULT;
      }
      throw new IllegalStateException("frame type should be FrameType.REQ, not "+frameType);
    }

    private Object[] getPeerAddress(HpnlBuffer hpnlBuffer) {
        //read peer address
        int peerAddrLen = hpnlBuffer.getInt();
        byte[] addrBytes = new byte[peerAddrLen];
        hpnlBuffer.get(addrBytes, 0, peerAddrLen);
        String ipPort[] = new String(addrBytes).split(":");
        Object address[] = new Object[]{ipPort[0], Integer.valueOf(ipPort[1])};
        return address;
    }

    private long getPeerProviderAddr(HpnlBuffer hpnlBuffer){
        //read peer name
        int peerNameLen = hpnlBuffer.getInt();
        byte[] peerBytes = new byte[peerNameLen];
        hpnlBuffer.get(peerBytes, 0, peerNameLen);
        ByteBuffer peerName = ByteBuffer.allocateDirect(peerNameLen);
        peerName.put(peerBytes);
        return connection.resolvePeerName(peerName);
    }
  }


  private int nextCqIndex() {
    cqIndex = (++cqIndex)%workerNum;
    if(cqIndex == 0){//0 for listening
        ++cqIndex;
    }
    return cqIndex;
  }

  private Connection createChildConnection(long remoteProviderAddress, Object[] peerAddress, int cqIndex,
                                           int sendCtxId, long tag, Handler recvCallback) {
    String destIp =  (String)peerAddress[0];
    int destPort = (Integer)peerAddress[1];
    if(log.isDebugEnabled()){
        log.debug("create child connection for remote {}:{}, remote provider address {}.", destIp, destPort,
                remoteProviderAddress);
    }

    //local address will be set in regCon callback
    long nativeConnHandler = this.get_con(destIp, destPort, null, 0,
            tag, remoteProviderAddress, cqIndex, sendCtxId, this.getNativeHandle());
    RdmConnection childConn = (RdmConnection) this.getConnection(nativeConnHandler);
    childConn.setCqIndex(cqIndex);
    childConn.setRecvCallback(recvCallback);
    return childConn;
  }
}
