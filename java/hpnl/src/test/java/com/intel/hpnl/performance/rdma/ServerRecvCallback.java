package com.intel.hpnl.performance.rdma;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.EqService;
import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.PooledByteBufAllocator;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.WritableByteChannel;
import java.nio.file.StandardOpenOption;

public class ServerRecvCallback implements Handler {
  private EqService eqService;
  private RdmaBuffer buffer;
  private File file;
  private FileChannel fileChannel;
  private long fileLen;
  private long count;

  public ServerRecvCallback(EqService eqService, File file) {
    this.eqService = eqService;
    this.file = file;
    this.fileLen = file.length();
    this.buffer = eqService.getRmaBuffer(4096 * 1024);
  }

  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    ByteBuffer buf = con.getRecvBuffer(rdmaBufferId).get(blockBufferSize);
    long length = buf.getLong();
    if(length == 0){
      length = 4096 * 1024;
    }
    readFile(length);
    RdmaBuffer sendBuffer = con.takeSendBuffer(true);
    ByteBuffer byteBufferTmp = ByteBuffer.allocate(24);
    byteBufferTmp.putLong(buffer.getAddress());
    byteBufferTmp.putLong(buffer.getRKey());
    byteBufferTmp.putLong(fileLen - count);

    byteBufferTmp.flip();
    sendBuffer.put(byteBufferTmp, (byte)0, 0);
    con.send(sendBuffer.getRawBuffer().remaining(), sendBuffer.getRdmaBufferId());
  }

  private void readFile(long length){
    try {
      if (this.fileChannel == null) {
        this.fileChannel = FileChannel.open(file.toPath(), StandardOpenOption.READ);
      }
      ByteBuffer rawBuffer = buffer.getRawBuffer();
      rawBuffer.clear();
      long size = fileChannel.transferTo(count, length, new WritableByteChannel() {
        @Override
        public int write(ByteBuffer src) throws IOException {
          int posBefore = rawBuffer.position();
          while (src.remaining() > 0) {
            rawBuffer.put(src);
          }
          return rawBuffer.position() - posBefore;
        }

        @Override
        public boolean isOpen() {
          return true;
        }

        @Override
        public void close() throws IOException {
        }
      });
      if ((length!=Long.MAX_VALUE) && size != length) {
        throw new RuntimeException(
                String.format("written bytes, %d, should be equal to requested length, %d", size, length));
      }
      count += size;
      if(count == fileLen){
        fileChannel.close();
      }
    }catch (Exception e){
      e.printStackTrace();
    }
  }
}
