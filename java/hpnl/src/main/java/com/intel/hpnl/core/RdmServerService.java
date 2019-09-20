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
    super(workNum, bufferNum, numRecvBuffers, bufferSize,  true);
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
    Connection connection = this.getConnection(this.listen(ip, port, this.getNativeHandle()));
    connection.setRecvCallback(new RdmServerReceiveHandler(connectedCallback, recvCallback));
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
          long peerConnectId = hpnlBuffer.getPeerConnectionId();
          if(connectionMap.containsKey(peerConnectId)){
              throw new IllegalStateException("child connection has been created already," +
                      " peer connection id is "+peerConnectId);
          }
          getPeerInfo(connection, peerConnectId, hpnlBuffer);
//      connection.setConnectedCallback(connectedCallback);
          //ack client request
          ackConnected(connection, peerConnectId);
          Connection childConnection = RdmServerService.this.createChildConnection(
                  connection.getProviderAddress(peerConnectId), connection.getPeerAddress(peerConnectId));
          connectionMap.put(peerConnectId, childConnection);
          connectedCallback.handle(childConnection, hpnlBuffer);
          return Handler.RESULT_DEFAULT;
      }
      throw new IllegalStateException("frame type should be FrameType.REQ, not "+frameType);
  }

  private void ackConnected(Connection connection, long peerConnectId){
      Object[] address = connection.getPeerAddress(peerConnectId);
      String addr = address[0] + ":" + address[1];
      HpnlBuffer buffer = connection.takeSendBuffer();
      if(buffer == null) {
          buffer = HpnlBufferAllocator.getBufferFromDefault(40 + HpnlRdmBuffer.METADATA_SIZE);
          ByteBuffer rawBuffer = buffer.getRawBuffer();
          rawBuffer.position(buffer.getMetadataSize());
          rawBuffer.put(addr.getBytes());
          buffer.insertMetadata(FrameType.ACK.id(), -1, rawBuffer.position());
          rawBuffer.flip();
          connection.sendBufferToId(buffer, rawBuffer.remaining(), peerConnectId);
      }else {
          ByteBuffer byteBuffer = ByteBuffer.allocate(40 + HpnlRdmBuffer.METADATA_SIZE);
          byteBuffer.put(addr.getBytes());
          byteBuffer.flip();
          buffer.putData(byteBuffer, FrameType.ACK.id(), -1L);
          connection.sendBufferToId(buffer, buffer.remaining(), peerConnectId);
      }

      if (log.isDebugEnabled()) {
          log.debug("acknowledging connection from " + addr);
      }
  }

    private void getPeerInfo(Connection connection, long peerConnectId, HpnlBuffer hpnlBuffer){
      //read peer address
      int peerAddrLen = hpnlBuffer.getInt();
      byte[] addrBytes = new byte[peerAddrLen];
      hpnlBuffer.get(addrBytes, 0, peerAddrLen);
      String ipPort[] = new String(addrBytes).split(":");
      Object address[] = new Object[]{ipPort[0], Integer.valueOf(ipPort[1])};
      connection.putPeerAddress(peerConnectId, address);
      //read peer name
      int peerNameLen = hpnlBuffer.getInt();
      byte[] peerBytes = new byte[peerNameLen];
      hpnlBuffer.get(peerBytes, 0, peerNameLen);
      ByteBuffer peerName = ByteBuffer.allocateDirect(peerNameLen);
      peerName.put(peerBytes);
      connection.putProviderAddress(peerConnectId, connection.resolvePeerName(peerName));
    }
  }

  private Connection createChildConnection(long srcProviderAddress, Object[] peerAddress) {
    cqIndex = (++cqIndex)%workerNum;
    if(cqIndex == 0){//0 for listening
        ++cqIndex;
    }
    long nativeConnHandler = this.get_con(localIp, String.valueOf(localPort), srcProviderAddress,
            cqIndex, this.getNativeHandle());
    RdmConnection childConn = (RdmConnection) this.getConnection(nativeConnHandler);
    int port = this.getFreePort();
    childConn.setAddrInfo((String)peerAddress[0], (Integer)peerAddress[1], localIp, port);
    childConn.setConnectionId(localIp, port);
    return childConn;
  }
}
