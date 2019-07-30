package com.intel.hpnl.core;

import java.nio.ByteBuffer;

public interface MemoryService {
    void setBuffer(ByteBuffer byteBuffer, int bufferSize, int bufferId);
}
