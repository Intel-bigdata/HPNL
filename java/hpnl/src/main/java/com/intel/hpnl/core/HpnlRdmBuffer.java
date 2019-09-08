package com.intel.hpnl.core;

import com.intel.hpnl.api.AbstractHpnlBuffer;
import com.intel.hpnl.api.Connection;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.ByteBuffer;

public class HpnlRdmBuffer extends AbstractHpnlBuffer {
  public static final int METADATA_SIZE = 8 + BASE_METADATA_SIZE;
  private static final Logger log = LoggerFactory.getLogger(HpnlRdmBuffer.class);
  private RdmConnection connection;
  private boolean released;
  private long oriConnectionId;

  public HpnlRdmBuffer(int bufferId, ByteBuffer byteBuffer, BufferType type) {
    super(bufferId, byteBuffer, type);
  }

  private void putMetadata(int srcSize, byte type, long seq) {
    this.byteBuffer.rewind();
    this.byteBuffer.limit(METADATA_SIZE + srcSize);
    this.byteBuffer.put(type);
    this.byteBuffer.putLong(this.connectionId);
    this.byteBuffer.putLong(seq);
  }

  @Override
  public void putData(ByteBuffer src, byte type, long seq) {
    this.putMetadata(src.remaining(), type, seq);
    this.byteBuffer.put(src);
    this.byteBuffer.flip();
  }

    @Override
    public void setConnectionId(long connectionId) {
        if(oriConnectionId == 0){
            oriConnectionId = connectionId;
        }
        this.connectionId = connectionId;
    }

  @Override
  public int getMetadataSize() {
    return METADATA_SIZE;
  }

  @Override
  public void insertMetadata(byte frameType, long seqId, int limit) {
    this.byteBuffer.position(0);
    this.byteBuffer.put(frameType);
    this.byteBuffer.putLong(this.connectionId);
    this.byteBuffer.putLong(seqId);
//    this.byteBuffer.limit(limit);
    this.byteBuffer.position(limit);
//    log.info("buffer-> {}, {} {}, {}, {}", getBufferId(), frameType, connectionId, seqId, limit);
  }

  @Override
  public void clear(){
      super.clear();
      released = false;
  }

    @Override
    public void clearState(){
      super.clearState();
      released = false;
    }

  public void setConnection(Connection connection) {
    this.connection = (RdmConnection) connection;
  }

  @Override
  public void release() {
      if (!released) {
          switch (getBufferType()) {
              case SEND:
                  connection.reclaimSendBuffer(getBufferId(), -1);
                  break;
              case RECV:
//        log.info("buffer id release: {}", getBufferId());
                  connection.reclaimRecvBuffer(getBufferId());
                  connectionId = oriConnectionId; // in case it's reused for sending in different connections
                  break;
              default:
                  throw new IllegalArgumentException("should not reach here: " + getBufferType());
          }
          released = true;
      }
  }
}
