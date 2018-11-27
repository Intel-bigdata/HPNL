package com.intel.hpnl.rma;

import java.nio.ByteBuffer;

import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;
import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.EqService;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;

public class ServerRecvCallback implements Handler {
  public ServerRecvCallback(EqService eqService, boolean is_server) {
    this.is_server = is_server;
    this.eqService = eqService;
    
    this.buf = eqService.getRmaBuffer(4096*1024);
    for (int i = 0; i < 4096*1024; i++) {
      buf.getRawBuffer().put((byte)i); 
    }
    buf.getRawBuffer().flip();
  }
  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    this.cur_buf = PooledByteBufAllocator.DEFAULT.directBuffer(4096*1024, 4096*1024);
    if (last_buf != null) {
      last_buf.release(); 
    }
    this.last_buf = cur_buf;
    this.buffer = eqService.regRmaBufferByAddress(this.cur_buf.nioBuffer(0, 4096*1024), this.cur_buf.memoryAddress(), 4096*1024);
    //for (int i = 0; i < 4096*1024; i++) {
    //  buffer.getRawBuffer().put((byte)i); 
    //}
    RdmaBuffer sendBuffer = con.getSendBuffer(true);
    ByteBuffer byteBufferTmp = ByteBuffer.allocate(16);
    //byteBufferTmp.putLong(this.cur_buf.memoryAddress());
    //byteBufferTmp.putLong(this.buffer.getRKey());
    byteBufferTmp.putLong(this.buf.getAddress());
    byteBufferTmp.putLong(this.buf.getRKey());

    byteBufferTmp.flip();
    sendBuffer.put(byteBufferTmp, (byte)0, 0, 0);
    con.send(sendBuffer.getRawBuffer().remaining(), sendBuffer.getRdmaBufferId());
  }
  private boolean is_server = false;
  private EqService eqService;
  private RdmaBuffer buffer;
  private RdmaBuffer buf;
  private ByteBuf last_buf = null;
  private ByteBuf cur_buf = null;
  private int count = 0;

}
