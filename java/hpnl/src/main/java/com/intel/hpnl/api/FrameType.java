package com.intel.hpnl.api;

/**
 * for NORMAL and SPLIT message, the send buffer may hold multiple messages of same type.
 * for STREAM message, the send buffer only hold one message.
 */
public enum FrameType {

  //each normal message can be sent within buffer, followed by message length(4) + message content
  //the send buffer may hold multiple normal messages
  NORMAL(0),

  //message size exceeds buffer size, need to send in splits, followed by id(8) + chunk id(4) +
  // total message length(8) + message split len(4) + message content
  //when receive, frame handler captures splits until all splits are received. then splits are forged to origin
  //message
  //the send buffer may hold multiple split messages
  SPLIT(1),

  //same as SPLIT cannot be sent within one buffer. the difference is each stream can be sank as long as all its
  //proceedings are sank.
  STREAM(2),

  ACK(100),

  REQ(101);

  private final byte id;

  FrameType(int id) {
    assert id < 128 : "Cannot have more than 128 frame types";
    this.id = (byte) id;
  }

  public byte id() {
    return id;
  }

  public static FrameType toFrameType(byte type){
    switch (type){
      case 0: return NORMAL;
      case 1: return SPLIT;
      case 2: return STREAM;
      case 100: return ACK;
      case 101: return REQ;
      default: throw new IllegalArgumentException("unknown frame type, "+type);
    }
  }
}
