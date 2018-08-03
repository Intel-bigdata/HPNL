#include "HPNL/FIConnection.h"

FIConnection::FIConnection(fid_fabric *fabric_, fi_info *info_, fid_domain *domain_, fid_cq* cq_, fid_wait *waitset_, BufMgr *recv_buf_mgr_, BufMgr *send_buf_mgr_, ConMgr* conMgr_, bool is_server) : info(info_), domain(domain_), conCq(cq_), recv_buf_mgr(recv_buf_mgr_), send_buf_mgr(send_buf_mgr_), waitset(waitset_), conMgr(conMgr_), server(is_server), read_callback(NULL), send_callback(NULL), shutdown_callback(NULL) {
  fi_endpoint(domain, info, &ep, NULL);
  
  struct fi_eq_attr eq_attr = {
    .size = 0,
    .flags = 0,
    .wait_obj = FI_WAIT_UNSPEC,
    .signaling_vector = 0,
    .wait_set = NULL
  };

  fi_eq_open(fabric_, &eq_attr, &conEq, &eqHandle);
  fi_ep_bind(ep, &conEq->fid, 0);

  fi_ep_bind(ep, &conCq->fid, FI_TRANSMIT | FI_RECV);
  
  fi_enable(ep);

  int size = 0;
  while (size < CON_MEM_SIZE) {
    fid_mr *mr;
    Chunk *ck = recv_buf_mgr->get();
    assert(!fi_mr_reg(domain, ck->buffer, BUFFER_SIZE, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr, NULL));
    ck->con = this;
    ck->mr = mr;
    assert(!fi_recv(ep, ck->buffer, BUFFER_SIZE, fi_mr_desc(mr), 0, ck));
    mr = NULL;
    recv_buffers.push_back(ck);
    size++;
  }
  size = 0;
  while (size < CON_MEM_SIZE) {
    fid_mr *mr;
    Chunk *ck = send_buf_mgr->get();
    assert(!fi_mr_reg(domain, ck->buffer, BUFFER_SIZE, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr, NULL));
    ck->con = this;
    ck->mr = mr;
    mr = NULL;
    send_buffers.push_back(ck);
    size++;
  }

  eqHandle.reset(new Handle(&conEq->fid, EQ_EVENT, conEq));
}

FIConnection::~FIConnection() {
  while (recv_buffers.size() > 0) {
    Chunk *ck = recv_buffers.back();
    fi_close(&((fid_mr*)ck->mr)->fid);
    recv_buffers.pop_back();
    recv_buf_mgr->add(ck->mid, ck);
  }
  while (send_buffers.size() > 0) {
    Chunk *ck = send_buffers.back();
    fi_close(&((fid_mr*)ck->mr)->fid);
    send_buffers.pop_back();
    send_buf_mgr->add(ck->mid, ck);
  }
  fi_close(&ep->fid);
  fi_close(&conEq->fid);
  if (server) {
    fi_freeinfo(info);
  }
}

void FIConnection::write(const char *buffer, int buffer_size, int mid) {
  // TODO: get send buffer
  Chunk *ck = send_buffers.back();
  send_buffers.pop_back();
  //memcpy(ck->buffer, buffer, buffer_size);
  memset(ck->buffer, '0', buffer_size);
  if (fi_send(ep, ck->buffer, buffer_size, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    // TODO: error handler
  }
}

void FIConnection::read(char *buffer, int buffer_size) {
  // TODO: buffer filter
}

void FIConnection::connect() {
  if (conMgr) {
    conMgr->push_event(ep, CONNECT, info->dest_addr);
  } else {
    fi_connect(ep, info->dest_addr, NULL, 0);
  }
}

void FIConnection::accept() {
  if (conMgr) {
    conMgr->push_event(ep, ACCEPT, NULL);
  } else {
    fi_accept(ep, NULL, 0);
  }
}

void FIConnection::shutdown() {
  if (conMgr) {
    conMgr->push_event(ep, SHUTDOWN, NULL);
  } else {
    assert(!fi_shutdown(ep, 0));
  }
}

void FIConnection::take_back_chunk(Chunk *ck) {
  send_buffers.push_back(ck);
}

void FIConnection::set_read_callback(Callback *callback) {
  read_callback = callback;
}

void FIConnection::set_send_callback(Callback *callback) {
  send_callback = callback;
}

void FIConnection::set_shutdown_callback(Callback *callback) {
  shutdown_callback = callback;
}

Callback* FIConnection::get_read_callback() {
  return read_callback;
}

Callback* FIConnection::get_send_callback() {
  return send_callback;
}

Callback* FIConnection::get_shutdown_callback() {
  return shutdown_callback;
}

fid* FIConnection::get_fid() {
  return &conEq->fid;
}

void FIConnection::activate_chunk(Chunk *ck) {
  ck->con = this;
  fi_recv(ep, ck->buffer, BUFFER_SIZE, fi_mr_desc((fid_mr*)ck->mr), 0, ck);
}

HandlePtr FIConnection::get_eqhandle() {
  return eqHandle;
}

