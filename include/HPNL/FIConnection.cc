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
  
  recv_chunk = recv_pool->get(1)[0];

  fi_enable(ep);
  fi_recv(ep, recv_chunk->buffer, BUFFER_SIZE, fi_mr_desc(recv_chunk->mr), 0, NULL);

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
  send_chunk = send_pool->pop(1)[0];
  memcpy(buffer, send_chunk->buffer, buffer_size);
  fi_send(ep, send_chunk->buffer, 4096, fi_mr_desc(send_chunk->mr), 0, NULL);
}

void FIConnection::read(char *buffer, int buffer_size) {

}

void FIConnection::connect() {
  assert(!fi_connect(ep, info->dest_addr, NULL, 0));
}

void FIConnection::accept() {
  assert(!fi_accept(ep, NULL, 0));
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
