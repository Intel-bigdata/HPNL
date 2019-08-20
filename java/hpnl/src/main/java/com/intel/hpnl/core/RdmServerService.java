package com.intel.hpnl.core;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.FrameType;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlBuffer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.ByteBuffer;

public class RdmServerService extends RdmService {
  private static final Logger log = LoggerFactory.getLogger(RdmServerService.class);

  public RdmServerService(int workNum, int bufferNum, int bufferSize, int ioRatio) {
    super(workNum, bufferNum, bufferSize,  ioRatio, true);
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
      HpnlBuffer buffer = connection.getRecvBuffer(bufferId);
      buffer.getRawBuffer().position(0);
      FrameType frameType = FrameType.toFrameType(buffer.getRawBuffer().get());

      switch (frameType) {
        case REQ:
          long connectId = buffer.getRawBuffer().getLong();
          getPeerInfo(connection, connectId, buffer, bufferSize);
          connection.setConnectedCallback(connectedCallback);
          //ack client request
          ackConnected(connection, connectId);
          connectedCallback.handle(connection, bufferId, bufferSize);
          return Handler.RESULT_DEFAULT;
        default:
          return recvCallback.handle(connection, bufferId, bufferSize);
      }
    }

      private void ackConnected(Connection connection, long connectId){
          Object[] address = connection.getPeerAddress(connectId);
          String addr = address[0] + ":" + address[1];
          HpnlBuffer buffer = connection.takeSendBuffer();
          if(buffer == null) {
              ByteBuffer tempBuffer = ByteBuffer.allocateDirect(40 + HpnlRdmBuffer.METADATA_SIZE);
              tempBuffer.put(FrameType.ACK.id());
              tempBuffer.putLong(connectId);
              tempBuffer.putLong(-1L);
              tempBuffer.put(addr.getBytes());
              tempBuffer.flip();
              connection.sendBufferTo(tempBuffer, tempBuffer.remaining(), connectId);
          }else {
              ByteBuffer byteBuffer = ByteBuffer.allocate(40);
              byteBuffer.put(addr.getBytes());
              byteBuffer.flip();
              buffer.putData(byteBuffer, FrameType.ACK.id(), -1L);
              connection.sendTo(buffer.remaining(), buffer.getBufferId(), connectId);
          }

          if (log.isDebugEnabled()) {
              log.debug("acknowledging connection from " + addr);
          }
      }

    private void getPeerInfo(Connection connection, long connectId, HpnlBuffer hpnlBuffer, int bufferSize){
      ByteBuffer msgBuffer = hpnlBuffer.parse(bufferSize);
      //read peer address
      int peerAddrLen = msgBuffer.getInt();
      byte[] addrBytes = new byte[peerAddrLen];
      msgBuffer.get(addrBytes, 0, peerAddrLen);
      String ipPort[] = new String(addrBytes).split(":");
      Object address[] = new Object[]{ipPort[0], Integer.valueOf(ipPort[1])};
      connection.putPeerAddress(connectId, address);
      //read peer name
      int peerNameLen = msgBuffer.getInt();
      byte[] peerBytes = new byte[peerNameLen];
      msgBuffer.get(peerBytes, 0, peerNameLen);
      ByteBuffer peerName = ByteBuffer.allocateDirect(peerNameLen);
      peerName.put(peerBytes);
      connection.putPeerName(connectId, peerName);
    }
  }
}
