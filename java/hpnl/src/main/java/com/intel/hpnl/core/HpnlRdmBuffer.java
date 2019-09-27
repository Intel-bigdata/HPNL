package com.intel.hpnl.core;

import com.intel.hpnl.api.AbstractHpnlBuffer;
import com.intel.hpnl.api.Connection;
import java.nio.ByteBuffer;

public class HpnlRdmBuffer extends AbstractHpnlBuffer {
  private RdmConnection connection;

  public HpnlRdmBuffer(int bufferId, ByteBuffer byteBuffer, BufferType type) {
    super(bufferId, byteBuffer, type);
  }


  public void setConnection(Connection connection) {
    this.connection = (RdmConnection) connection;
  }

  @Override
  public void release() {
      switch (getBufferType()) {
          case SEND:
              connection.reclaimSendBuffer(getBufferId(), -1);
              break;
          case RECV:
              connection.reclaimRecvBuffer(getBufferId());
              break;
          default:
              throw new IllegalArgumentException("should not reach here: " + getBufferType());
      }
  }
}
