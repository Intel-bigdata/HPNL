package com.intel.hpnl.core.mappedbuffer;

import com.intel.hpnl.api.Connection;
import com.intel.hpnl.api.FrameType;
import com.intel.hpnl.api.Handler;
import com.intel.hpnl.api.HpnlBuffer;

import java.nio.ByteBuffer;

public class RecvCallback implements Handler {

  private ByteBuffer peerName;

  public RecvCallback() {}

  public int handle(Connection con, int bufferId, int blockBufferSize) {
    HpnlBuffer recvBuffer = con.getRecvBuffer(bufferId);
    assert(recvBuffer != null);
    ByteBuffer recvByteBuffer = recvBuffer.parse(blockBufferSize);

    HpnlBuffer buffer = con.takeSendBuffer();
    buffer.clear();
    ByteBuffer rawBuffer = buffer.getRawBuffer();
    rawBuffer.position(buffer.getMetadataSize());
    rawBuffer.put(recvByteBuffer);
    buffer.insertMetadata(FrameType.NORMAL.id());
    rawBuffer.flip();
//    con.releaseRecvBuffer(bufferId);
    return Handler.RESULT_DEFAULT;
  }

  private long count = 0;
  private long startTime;
  private long endTime;
  private float total_time = 0;
  private float latency = 0;
  private float throughput = 0;

  private boolean is_server = false;
  private int interval;
  private int msgSize;

}
