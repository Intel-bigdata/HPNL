#ifndef CONNECTION_H
#define CONNECTION_H

#include "HPNL/BufMgr.h"
#include "HPNL/Callback.h"

class Connection {
  public:
    virtual           ~Connection() = default;;
    virtual int       init() = 0;
    // Message Interface 
    // a. msg endpoint
    virtual int       send(Chunk*) { return 0; }
    virtual int       send(int, int) { return 0; }
    virtual int       sendBuf(const char*, int) { return 0; }
    virtual int       sendTo(int, int, const char*) { return 0; }
    virtual int       sendBufTo(const char*, int, const char*) { return 0; }
    virtual fi_addr_t recv(const char*, int) { return 0; }

    // b. rdm endpoint
    virtual char*     get_peer_name() { return nullptr; }
    virtual void      decode_peer_name(void*, char*, int) {}
    virtual char*     decode_buf(void *) { return nullptr; }
    virtual Chunk*    encode(void*, int, char*) { return nullptr; }

    virtual void      activate_send_chunk(Chunk*) {}
    virtual int       activate_recv_chunk(Chunk*) { return 0; }
    virtual void      set_recv_callback(Callback*) {}
    virtual void      set_send_callback(Callback*) {}
    virtual Callback* get_recv_callback() = 0;
    virtual Callback* get_send_callback() = 0;

    // Remote Memory Access Interface
    virtual int       read(int, int, uint64_t, uint64_t, uint64_t) { return 0; }

};

#endif
