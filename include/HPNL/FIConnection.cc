#include "FIConnection.h"

FIConnection::FIConnection(fid_fabric *fabric_, fi_info *info_, fid_domain *domain_, Mempool *rpool, Mempool *spool, bool is_server) : info(info_), domain(domain_), recv_pool(rpool), send_pool(spool), server(is_server) {
  fi_endpoint(domain, info, &ep, NULL);

  struct fi_eq_attr eq_attr = {
    .size = 0,
    .flags = 0,
    .wait_obj = FI_WAIT_UNSPEC,
    .signaling_vector = 0,
    .wait_set = NULL
  };

  fi_eq_open(fabric_, &eq_attr, &conEq, &cmHandle);
  fi_ep_bind(ep, &conEq->fid, 0);

  struct fi_cq_attr cq_attr = {
    .size = 0,
    .flags = 0,
    .format = FI_CQ_FORMAT_MSG,
    .wait_obj = FI_WAIT_UNSPEC,
    .signaling_vector = 0,
    .wait_cond = FI_CQ_COND_NONE,
    .wait_set = NULL
  };

  fi_cq_open(domain, &cq_attr, &conCq, &cqHandle);
  fi_ep_bind(ep, &conCq->fid, FI_TRANSMIT | FI_RECV);
  

  fi_enable(ep);
  std::vector<Chunk*> vec = std::move(recv_pool->get(CON_MEMPOOL_SIZE));
  for (Chunk *ck : vec) {
    fi_recv(ep, ck->buffer, BUFFER_SIZE, fi_mr_desc(ck->mr), 0, ck);
  }
  cmHandle.reset(new Handle(&conEq->fid, EQ_EVENT, conEq));
}

FIConnection::~FIConnection() {
  fi_close(&ep->fid);
  fi_close(&conCq->fid);
  fi_close(&conEq->fid);
  if (server) {
    fi_freeinfo(info);
  }
}

void FIConnection::write(char *buffer, int buffer_size) {
  std::vector<Chunk*> vec = std::move(send_pool->pop(1));
  Chunk *ck = vec[0];
  memcpy(ck->buffer, buffer, buffer_size);
  if (fi_send(ep, ck->buffer, BUFFER_SIZE, fi_mr_desc(ck->mr), 0, ck)) {
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

HandlePtr FIConnection::connected() {
  cqHandle.reset(new Handle(&conCq->fid, CQ_EVENT, conCq));
  return cqHandle;
}

fid* FIConnection::get_fid() {
  return &conEq->fid;
}

HandlePtr FIConnection::get_cmhandle() {
  return cmHandle;
}

HandlePtr FIConnection::get_cqhandle() {
  return cqHandle;
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
  fi_recv(ep, ck->buffer, BUFFER_SIZE, fi_mr_desc(ck->mr), 0, ck);
}
