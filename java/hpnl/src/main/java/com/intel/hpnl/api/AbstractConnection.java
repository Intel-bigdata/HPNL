package com.intel.hpnl.api;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public abstract class AbstractConnection implements Connection {
  protected HpnlService service;
  private Handler connectedCallback = null;
  private Handler recvCallback = null;
  private Handler sendCallback = null;
  private Handler readCallback = null;
  private List<Handler> shutdownCallbacks = new ArrayList();
  private Handler generalCallback;
  private Map<Integer, HpnlBuffer> sendBufferMap;
  private Queue<HpnlBuffer> sendBufferList;
  private Map<Integer, HpnlBuffer> recvBufferMap;
  private String destAddr;
  private int destPort;
  private String srcAddr;
  private int srcPort;
  private boolean connected;
  protected int cqIndex;
  protected long connectId;
  private static final Logger log = LoggerFactory.getLogger(AbstractConnection.class);

  protected AbstractConnection(long nativeCon, HpnlService service, long connectId) {
    this.service = service;
    this.sendBufferMap = new HashMap();
    this.recvBufferMap = new HashMap();
    this.sendBufferList = new LinkedList();
    this.initialize(nativeCon);
    this.cqIndex = this.getCqIndexFromNative(this.getNativeHandle());
    this.connected = true;
    if (connectId == 0L) {
      this.connectId = service.getNewConnectionId();
    } else {
      this.connectId = connectId;
    }

    this.shutdownCallbacks.add(new AbstractConnection.InternalShutdownCallback());
  }

  public int getCqIndex() {
    return this.cqIndex;
  }

  public long getConnectionId() {
    return this.connectId;
  }

  protected abstract void initialize(long var1);

  protected abstract int getCqIndexFromNative(long var1);

  protected abstract long getNativeHandle();

  public Handler getConnectedCallback() {
    return this.connectedCallback;
  }

  public void setConnectedCallback(Handler callback) {
    this.connectedCallback = callback;
  }

  public Handler getRecvCallback() {
    return this.recvCallback;
  }

  public void setRecvCallback(Handler callback) {
    this.recvCallback = callback;
  }

  public Handler getSendCallback() {
    return this.sendCallback;
  }

  public void setSendCallback(Handler callback) {
    this.sendCallback = callback;
  }

  public void setReadCallback(Handler callback) {
    this.readCallback = callback;
  }

  public List<Handler> getShutdownCallbacks() {
    return this.shutdownCallbacks;
  }

  public void addShutdownCallback(Handler callback) {
    this.shutdownCallbacks.add(callback);
  }

  public void setGeneralEventCallback(Handler callback) {
    this.generalCallback = callback;
  }

  public Handler getGeneralCallback() {
    return this.generalCallback;
  }

  protected void reclaimSendBuffer(int bufferId) {
    HpnlBuffer buffer = (HpnlBuffer)this.sendBufferMap.get(bufferId);
    if (buffer == null) {
      throw new IllegalStateException("buffer not found with id: " + bufferId);
    } else {
      this.sendBufferList.offer(buffer);
    }
  }

  public void pushSendBuffer(HpnlBuffer buffer) {
    synchronized(this) {
      this.sendBufferMap.put(buffer.getBufferId(), buffer);
      this.sendBufferList.offer(buffer);
    }
  }

  public void pushRecvBuffer(HpnlBuffer buffer) {
    synchronized(this) {
      this.recvBufferMap.put(buffer.getBufferId(), buffer);
    }
  }

  public HpnlBuffer takeSendBuffer() {
    return (HpnlBuffer)this.sendBufferList.poll();
  }

  public HpnlBuffer getSendBuffer(int bufferId) {
    return (HpnlBuffer)this.sendBufferMap.get(bufferId);
  }

  public HpnlBuffer getRecvBuffer(int bufferId) {
    return (HpnlBuffer)this.recvBufferMap.get(bufferId);
  }

  protected int handleCallback(int eventType, int bufferId, int bufferSize) {
    int e;
    switch(eventType) {
      case 2:
        e = this.executeCallback(this.connectedCallback, bufferId, 0);
        break;
      case 4:
        e = this.executeCallback(this.readCallback, bufferId, bufferSize);
        break;
      case 16:
        e = this.executeCallback(this.recvCallback, bufferId, bufferSize);
        break;
      case 32:
        e = this.executeCallback(this.sendCallback, bufferId, bufferSize);
        if (e == 1) {
          this.reclaimSendBuffer(bufferId);
        }
        break;
      case 1024:
        e = this.executeCallbacks(this.shutdownCallbacks, bufferId, 0);
        break;
      default:
        e = 1;
    }

    this.executeCallback(this.generalCallback, -1, -1);
    return e;
  }

  private int executeCallback(Handler handler, int bufferId, int bufferSize) {
    if (handler == null) {
      return 1;
    } else {
      try {
        return handler.handle(this, bufferId, bufferSize);
      } catch (Throwable var5) {
        log.error("failed to execute callback " + handler, var5);
        return 1;
      }
    }
  }

  private int executeCallbacks(List<Handler> callbacks, int bufferId, int bufferSize) {
    int ret = 1;
    Iterator var5 = callbacks.iterator();

    while(var5.hasNext()) {
      Handler callback = (Handler)var5.next();
      int tmp = this.executeCallback(callback, bufferId, bufferSize);
      if (tmp == 0) {
        ret = tmp;
      }
    }

    return ret;
  }

  public void setAddrInfo(String destAddr, int destPort, String srcAddr, int srcPort) {
    this.destAddr = destAddr;
    this.destPort = destPort;
    this.srcAddr = srcAddr;
    this.srcPort = srcPort;
  }

  public String getDestAddr() {
    return this.destAddr;
  }

  public int getDestPort() {
    return this.destPort;
  }

  public String getSrcAddr() {
    return this.srcAddr;
  }

  public int getSrcPort() {
    return this.srcPort;
  }

  public int sendTo(int bufferSize, int bufferId, ByteBuffer peerName) {
    throw new UnsupportedOperationException();
  }

  public ByteBuffer getLocalName() {
    throw new UnsupportedOperationException();
  }

  public void shutdown() {
    this.shutdown(true);
  }

  private void shutdown(boolean proactive) {
    if (this.connected) {
      synchronized(this) {
        if (this.connected) {
          this.doShutdown(proactive);
          this.connected = false;
          if (log.isDebugEnabled()) {
            log.debug("connection {} with CQ index {} closed.", this.connectId, this.cqIndex);
          }

        }
      }
    }
  }

  protected abstract void doShutdown(boolean var1);

  private class InternalShutdownCallback implements Handler {
    private InternalShutdownCallback() {
    }

    public int handle(Connection connection, int bufferId, int bufferSize) {
      AbstractConnection.this.shutdown(false);
      return 1;
    }
  }
}
