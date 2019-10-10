package com.intel.hpnl.core;

import com.intel.hpnl.api.*;

import java.util.List;
import java.util.concurrent.atomic.AtomicLong;

public class RdmHpnlService implements HpnlService {
  private RdmService service;
  private boolean server;
  private AtomicLong nextConnectionId = new AtomicLong(1L);

  public RdmHpnlService(int numThreads, int numBuffers, int numRecvBuffers, int bufferSize, boolean server) {
    if(bufferSize < HpnlBufferAllocator.BUFFER_LARGE){
      throw new IllegalArgumentException("buffer size should be no less than "+HpnlBufferAllocator.BUFFER_LARGE);
    }
    if (server) {
      this.service = (new RdmServerService(numThreads, numBuffers, numRecvBuffers, bufferSize)).init();
    } else {
      this.service = (new RdmService(numThreads, numBuffers, numRecvBuffers, bufferSize)).init();
    }
    this.service.setHpnlService(this);
    this.server = server;
  }

  @Override
  public int bind(String hostname, int port, int cqIndex, Handler connectedCallback, Handler recvCallback) {
    return this.service.connect(hostname, port, cqIndex, connectedCallback, recvCallback);
  }

  @Override
  public int connect(String hostname, int port, int cqIndex, Handler connectedCallback, Handler recvCallback) {
    return this.service.connect(hostname, port, cqIndex, connectedCallback, recvCallback);
  }

  @Override
  public void stop() {
    this.service.stop();
  }

  @Override
  public HpnlBuffer getRecvBuffer(int bufferId){
    return service.getRecvBuffer(bufferId);
  }

  @Override
  public EventTask getCqTask(int cqIndex) {
    return service.getEventTask(cqIndex);
  }

  public RdmService getRdmService(){
    return service;
  }

  @Override
  public List<EventTask> getCqTasks() {
    return service.getCqTasks();
  }

  @Override
  public long getNewConnectionId() {
    return this.nextConnectionId.getAndIncrement();
  }

  @Override
  public void removeNativeConnection(long nativeConnectionId, long connEq, boolean proactive) {
    this.service.removeConnection(nativeConnectionId, connEq, proactive);
  }

  @Override
  public void ackConnected(Connection connection){
    service.ackConnected(connection);
  }

  @Override
  public boolean isServer() {
    return this.server;
  }

  @Override
  public int getFreePort() {
    return service.getFreePort();
  }

  @Override
  public void reclaimPort(int port) {
    service.reclaimPort(port);
  }

  @Override
  public Connection getConnection(){
    return service.getConnection();
  }

  @Override
  public EndpointType getEndpointType() {
    return EndpointType.RDM;
  }
}
