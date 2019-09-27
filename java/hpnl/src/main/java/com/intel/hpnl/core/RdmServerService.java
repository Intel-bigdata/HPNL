package com.intel.hpnl.core;

import com.intel.hpnl.api.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

public class RdmServerService extends RdmService {
  private int cqIndex = 0;
  private Map<Long, Connection> connectionMap = new HashMap<>();

  private static final Logger log = LoggerFactory.getLogger(RdmServerService.class);

  public RdmServerService(int workNum, int bufferNum, int numRecvBuffers, int bufferSize) {
    super(workNum, bufferNum, numRecvBuffers, bufferSize, true);
    if(workNum < 2){
        throw new IllegalArgumentException("work number should be at least 2 for server");
    }
  }

  @Override
  public RdmService init() {
    return init(HpnlConfig.getInstance().getSrvCtxNum());
  }

  @Override
  public int connect(String ip, String port, int cqIndex, Handler connectedCallback, Handler recvCallback) {
    localIp = ip;
    localPort = Integer.valueOf(port);
    RdmConnection conn = (RdmConnection) this.getConnection(this.listen(ip, port, this.getNativeHandle()));
    conn.setCqIndex(cqIndex);
    setConnection(conn);
    conn.setRecvCallback(new RdmServerReceiveHandler(connectedCallback, recvCallback));
    return 1;
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
          Connection childConnection = RdmServerService.this.createChildConnection(provAddr, address,
                  cqIndex, recvCallback);
          ackConnected(childConnection, address, cqIndex);
          connectionMap.put(peerConnectId, childConnection);
          connectedCallback.handle(childConnection, hpnlBuffer);
          return Handler.RESULT_DEFAULT;
      }
      throw new IllegalStateException("frame type should be FrameType.REQ, not "+frameType);
    }

    private void ackConnected(Connection connection, Object[] address, int cqIndex){
      String addr = address[0] + ":" + address[1];
      int dataSize = 60;
      HpnlBuffer buffer = connection.takeSendBuffer();
      if(buffer == null) {
          buffer = HpnlBufferAllocator.getBufferFromDefault(dataSize + AbstractHpnlBuffer.BASE_METADATA_SIZE);
          ByteBuffer rawBuffer = buffer.getRawBuffer();
          rawBuffer.position(buffer.getMetadataSize());
          rawBuffer.putInt(cqIndex);
          rawBuffer.putInt(addr.getBytes().length);
          rawBuffer.put(addr.getBytes());
          buffer.insertMetadata(FrameType.ACK.id());
          rawBuffer.flip();
          connection.sendBuffer(buffer, rawBuffer.remaining());
      }else {
          ByteBuffer byteBuffer = ByteBuffer.allocate(dataSize + AbstractHpnlBuffer.BASE_METADATA_SIZE);
          byteBuffer.putInt(cqIndex);
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
                                           Handler recvCallback) {
    if(log.isDebugEnabled()){
        log.debug("create child connection for remote address, "+remoteProviderAddress);
    }

    long nativeConnHandler = this.get_con(localIp, String.valueOf(localPort), remoteProviderAddress,
            cqIndex, 0, this.getNativeHandle());
    RdmConnection childConn = (RdmConnection) this.getConnection(nativeConnHandler);
    childConn.setCqIndex(cqIndex);
    //local addr was set when registered connection
    childConn.setAddrInfo((String)peerAddress[0], (Integer)peerAddress[1], childConn.getSrcAddr(), childConn.getSrcPort());
    childConn.setRecvCallback(recvCallback);
    return childConn;
  }
}
