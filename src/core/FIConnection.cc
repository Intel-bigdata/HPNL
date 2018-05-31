#include "FIConnection.h"

FIConnection::FIConnection(fid_fabric *fabric_, fi_info *info_, fid_domain *domain_, fid_cq* cq_, fid_wait *waitset_, Mempool *rpool, Mempool *spool, bool is_server) : info(info_), domain(domain_), conCq(cq_), waitset(waitset_), recv_pool(rpool), send_pool(spool), server(is_server), read_callback(NULL) {
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
  std::vector<Chunk*> vec = std::move(recv_pool->get(CON_MEMPOOL_SIZE));
  for (Chunk *ck : vec) {
    ck->con = this;
    fi_recv(ep, ck->buffer, BUFFER_SIZE, fi_mr_desc(ck->mr), 0, ck);
  }
  eqHandle.reset(new Handle(&conEq->fid, EQ_EVENT, conEq));
}

FIConnection::~FIConnection() {
  fi_close(&ep->fid);
  fi_close(&conEq->fid);
  if (server) {
    fi_freeinfo(info);
  }
}

void FIConnection::write(char *buffer, int buffer_size) {
  std::vector<Chunk*> vec = std::move(send_pool->pop(1));
  Chunk *ck = vec[0];
  memcpy(ck->buffer, buffer, buffer_size);
  ck->con = this;
  if (fi_send(ep, ck->buffer, buffer_size, fi_mr_desc(ck->mr), 0, ck)) {
    std::vector<Chunk*> send_vec;
    send_vec.push_back(ck);
    send_chunk_to_pool(std::move(send_vec)); 
  }
}

void FIConnection::read(char *buffer, int buffer_size) {
  // TODO: buffer filter
}

void FIConnection::connect() {
  assert(!fi_connect(ep, info->dest_addr, NULL, 0));
}

void FIConnection::accept() {
  assert(!fi_accept(ep, NULL, 0));
}

void FIConnection::shutdown() {
  fi_shutdown(ep, 0); 
}

void FIConnection::set_read_callback(Callback *callback) {
  read_callback = callback;
}

Callback* FIConnection::get_read_callback() {
  return read_callback;
}

HandlePtr FIConnection::connected() {
  return NULL;
}

fid* FIConnection::get_fid() {
  return &conEq->fid;
}

HandlePtr FIConnection::get_eqhandle() {
  return eqHandle;
}

Mempool* FIConnection::get_rpool() {
  return recv_pool;
}

Mempool* FIConnection::get_spool() {
  return send_pool;
}

void FIConnection::send_chunk_to_pool(std::vector<Chunk*> vec) {
  send_pool->push(std::move(vec));
}

void FIConnection::reactivate_chunk(Chunk *ck) {
  ck->con = this;
  fi_recv(ep, ck->buffer, BUFFER_SIZE, fi_mr_desc(ck->mr), 0, ck);
}
