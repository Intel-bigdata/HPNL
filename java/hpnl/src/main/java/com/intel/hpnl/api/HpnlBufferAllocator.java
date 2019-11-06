package com.intel.hpnl.api;

public interface HpnlBufferAllocator {

    int BUFFER_METADATA_SIZE = 1;

    HpnlBuffer getBuffer(int size);

    HpnlBuffer getBuffer(boolean large);

    void clear();
}
