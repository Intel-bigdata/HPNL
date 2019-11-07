package com.intel.hpnl.api;

import io.netty.buffer.ByteBuf;

public interface HpnlRdmaBuffer extends HpnlBuffer {

    ByteBuf getRawRdmaBuffer();

    @Override
    default BufferType getBufferType(){
        return BufferType.RDMA;
    }

    @Override
    default long getMemoryAddress(){
        return getRawRdmaBuffer().memoryAddress();
    }
}
