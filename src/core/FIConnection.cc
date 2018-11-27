#include "HPNL/FIConnection.h"
#include "HPNL/FIStack.h"

FIConnection::FIConnection(FIStack *stack_, fid_fabric *fabric_, fi_info *info_, fid_domain *domain_, fid_cq* cq_, fid_wait *waitset_, BufMgr *recv_buf_mgr_, BufMgr *send_buf_mgr_, bool is_server, int buffer_num) : stack(stack_), info(info_), domain(domain_), conCq(cq_), recv_buf_mgr(recv_buf_mgr_), send_buf_mgr(send_buf_mgr_), waitset(waitset_), server(is_server), read_callback(NULL), send_callback(NULL), shutdown_callback(NULL) {
  assert(!fi_endpoint(domain, info, &ep, NULL));
  
  struct fi_eq_attr eq_attr = {
    .size = 0,
    .flags = 0,
    .wait_obj = FI_WAIT_UNSPEC,
    .signaling_vector = 0,
    .wait_set = NULL
  };

  if (fi_eq_open(fabric_, &eq_attr, &conEq, &eqHandle)) {
    std::cout << "eq open error " << errno << std::endl; 
  }
  assert(!fi_ep_bind(ep, &conEq->fid, 0));

  assert(!fi_ep_bind(ep, &conCq->fid, FI_TRANSMIT | FI_RECV));
  
  assert(!fi_enable(ep));
  int size = 0;
  while (size < buffer_num*2) {
    fid_mr *mr;
    Chunk *ck = recv_buf_mgr->get();
    assert(ck->buffer);
    assert(!fi_mr_reg(domain, ck->buffer, ck->capacity, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr, NULL));
    ck->con = this;
    ck->mr = mr;
    assert(!fi_recv(ep, ck->buffer, ck->capacity, fi_mr_desc(mr), 0, ck));
    mr = NULL;
    recv_buffers.push_back(ck);
    size++;
  }
  size = 0;
  while (size < buffer_num) {
    fid_mr *mr;
    Chunk *ck = send_buf_mgr->get();
    assert(!fi_mr_reg(domain, ck->buffer, ck->capacity, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr, NULL));
    ck->con = this;
    ck->mr = mr;
    mr = NULL;
    send_buffers.push_back(ck);
    send_buffers_map.insert(std::pair<int, Chunk*>(ck->rdma_buffer_id, ck));
    size++;
  }

  eqHandle.reset(new Handle(&conEq->fid, EQ_EVENT, conEq));
}

FIConnection::~FIConnection() {
  for (auto buffer: send_buffers_map) {
    Chunk *ck = buffer.second;
    fi_close(&((fid_mr*)ck->mr)->fid);
    send_buf_mgr->add(ck->rdma_buffer_id, ck);
  }
  while (recv_buffers.size() > 0) {
    Chunk *ck = recv_buffers.back();
    fi_close(&((fid_mr*)ck->mr)->fid);
    recv_buffers.pop_back();
    recv_buf_mgr->add(ck->rdma_buffer_id, ck);
  }
  shutdown();
  fi_close(&ep->fid);
  fi_close(&conEq->fid);
  if (server) {
    fi_freeinfo(info);
  }
}

void FIConnection::send(const char *buffer, int block_buffer_size, int rdma_buffer_id, int block_buffer_id, long seq) {
  // TODO: get send buffer
  Chunk *ck = send_buffers.back();
  assert(ck->buffer);
  send_buffers.pop_back();
  memcpy(ck->buffer, buffer, block_buffer_size);
  if (fi_send(ep, ck->buffer, block_buffer_size, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    // TODO: error handler
  }
}

void FIConnection::send(int block_buffer_size, int rdma_buffer_id) {
  Chunk *ck = send_buffers_map[rdma_buffer_id];
  if (fi_send(ep, ck->buffer, block_buffer_size, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    // TODO: error handler
  }
}

void FIConnection::recv(char *buffer, int buffer_size) {
  // TODO: buffer filter
}

int FIConnection::read(int rdma_buffer_id, int local_offset, uint64_t len, uint64_t remote_addr, uint64_t remote_key) {
  Chunk *ck = stack->get_rma_chunk(rdma_buffer_id);
  ck->con = this;
  return fi_read(ep, (char*)ck->buffer+local_offset, len, fi_mr_desc((fid_mr*)ck->mr), 0, remote_addr, remote_key, ck);
}

void FIConnection::connect() {
  std::cout << "connect." << std::endl;
  assert(!fi_connect(ep, info->dest_addr, NULL, 0));
}

void FIConnection::accept() {
  std::cout << "accept." << std::endl;
  assert(!fi_accept(ep, NULL, 0));
}

void FIConnection::shutdown() {
  std::cout << "shutdown." << std::endl;
  assert(!fi_shutdown(ep, 0));
}

void FIConnection::take_back_chunk(Chunk *ck) {
  send_buffers.push_back(ck);
}

std::vector<Chunk*> FIConnection::get_send_buffer() {
  return send_buffers;
}

void FIConnection::set_recv_callback(Callback *callback) {
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
  fi_recv(ep, ck->buffer, ck->capacity, fi_mr_desc((fid_mr*)ck->mr), 0, ck);
}

HandlePtr FIConnection::get_eqhandle() {
  return eqHandle;
}

