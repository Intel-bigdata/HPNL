package com.intel.hpnl.performance.rdma;

import com.intel.hpnl.core.Connection;
import com.intel.hpnl.core.Handler;
import com.intel.hpnl.core.RdmaBuffer;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.Pipe;
import java.nio.file.StandardOpenOption;

public class ClientReadCallback implements Handler {
  private File destFile;
  private FileChannel fileChannel;
  private Thread recvThread;
  private Pipe.SinkChannel sink;
  private Pipe.SourceChannel source;
  private volatile long fileLen;
  private long pos;

  public ClientReadCallback(File file) {
    this.destFile = file;

    try {
      if(!destFile.exists()){
        destFile.createNewFile();
      }
      fileChannel = FileChannel.open(destFile.toPath(), StandardOpenOption.WRITE);
      Pipe pipe = Pipe.open();
      source = pipe.source();
      sink = pipe.sink();
    }catch (IOException e){
      throw new RuntimeException(e);
    }
  }

  public synchronized void handle(Connection con, int rdmaBufferId, int blockBufferSize) {
    ByteBuffer byteBuffer = con.getRmaBuffer(rdmaBufferId);
    if (count == 0) {
      startTime = System.currentTimeMillis();
      recvThread = new Thread(new Runnable() {
        @Override
        public void run() {
          try {
            long size = fileChannel.transferFrom(source, 0, fileLen);
            if(size != fileLen){
              throw new IOException("received: "+size+", expected: "+fileLen);
            }
            source.close();
            sink.close();
            endTime = System.currentTimeMillis();
            System.out.println(endTime - startTime);
          }catch (IOException e){
            throw new RuntimeException(e);
          }
        }
      });
      recvThread.start();
    }
    count++;

    try {
      sink.write(byteBuffer);
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
  }
  private int count = 0;
  private volatile long startTime;
  private volatile long endTime;
  private float totally_time = 0;

  public void setFileLen(long fileLen) {
    this.fileLen = fileLen;
  }
}
