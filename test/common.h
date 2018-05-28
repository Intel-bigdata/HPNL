#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <cstdlib>

#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_rma.h>

#define FIVER FI_VERSION(1, 1)

struct keys {
  uint64_t rkey;
  uint64_t addr;
};

int count = 1000000;
void *recv_buff;
void *send_buff;
struct keys send_keys;
struct keys recv_keys;
size_t buff_size;
struct fi_info *hints, *info;
struct fid_fabric *fabric;
struct fid_domain *domain;
struct fi_eq_attr eq_attr = {
  .size = 0,
  .flags = 0,
  .wait_obj = FI_WAIT_UNSPEC,
  .signaling_vector = 0,
  .wait_set = NULL
};
struct fid_eq *eq;
struct fid_cq *cq;
struct fid_mr *recv_mr;
struct fid_mr *send_mr;
struct fid_pep *pep;
struct fid_ep *ep;
struct fi_eq_cm_entry entry;
uint32_t event;

int common_init(const char *addr, uint64_t flags, size_t size) {
 
  buff_size = size;
  hints = fi_allocinfo();
  hints->addr_format = FI_SOCKADDR_IN;
  hints->ep_attr->type = FI_EP_MSG;
  hints->domain_attr->mr_mode = FI_MR_BASIC;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT | FI_LOCAL_MR | FI_RX_CQ_DATA;

  fi_getinfo(FIVER, addr, "12345", flags, hints, &info);

  fi_fabric(info->fabric_attr, &fabric, NULL);

  fi_eq_open(fabric, &eq_attr, &eq, NULL);

  fi_domain(fabric, info, &domain, NULL);

  fid_poll *pollset;
  fi_poll_attr attr = {};

  //assert(0 == fi_poll_open(domain, &attr, &pollset));

  struct fi_cq_attr cq_attr = {
    .size = 0,
    .flags = 0,
    .format = FI_CQ_FORMAT_MSG,
    .wait_obj = FI_WAIT_UNSPEC,
    .signaling_vector = 0,
    .wait_cond = FI_CQ_COND_NONE,
    .wait_set = NULL
  };

  fi_cq_open(domain, &cq_attr, &cq, NULL);

  recv_buff = malloc(buff_size);
  fi_mr_reg(domain, recv_buff, buff_size, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &recv_mr, NULL);
  send_buff = malloc(buff_size);
  fi_mr_reg(domain, send_buff, buff_size, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &send_mr, NULL);

  return 0;
}

void server_connect() {

  fi_passive_ep(fabric, info, &pep, NULL);

  fi_pep_bind(pep, &eq->fid, 0);

  fi_listen(pep);

  struct fi_eq_cm_entry entry;
  uint32_t event;

  fi_eq_sread(eq, &event, &entry, sizeof(entry), -1, 0);

  fi_endpoint(domain, entry.info, &ep, NULL);

  fi_ep_bind(ep, &eq->fid, 0);

  fi_ep_bind(ep, &cq->fid, FI_TRANSMIT | FI_RECV);

  fi_enable(ep);

  fi_recv(ep, recv_buff, buff_size, fi_mr_desc(recv_mr), 0, NULL);

  fi_accept(ep, NULL, 0);

  fi_eq_sread(eq, &event, &entry, sizeof(entry), -1, 0);
  
  assert(event == FI_CONNECTED);

  printf("connected.\n");
}

void client_connect() {
    
  fi_endpoint(domain, info, &ep, NULL);

  fi_ep_bind(ep, &eq->fid, 0);

  fi_ep_bind(ep, &cq->fid, FI_TRANSMIT | FI_RECV);

  fi_enable(ep);

  fi_recv(ep, recv_buff, buff_size, fi_mr_desc(recv_mr), 0, NULL);
  fi_send(ep, send_buff, buff_size, fi_mr_desc(send_mr), 0, NULL);


  fi_connect(ep, info->dest_addr, NULL, 0);
  
  fi_eq_sread(eq, &event, &entry, sizeof(entry), -1, 0);

  assert(event == FI_CONNECTED);

  printf("connected.\n");
}

void send_key() {
  send_keys.rkey = fi_mr_key(send_mr);
  send_keys.addr = (uint64_t)send_buff;
  memcpy(send_buff, &send_keys, sizeof(send_keys));
  fi_send(ep, send_buff, sizeof(send_keys), fi_mr_desc(send_mr), 0, NULL);
  struct fi_cq_msg_entry comp;
  fi_cq_sread(cq, &comp, 1, NULL, -1);
  assert(comp.flags & FI_SEND);
  memset(send_buff, '0', buff_size);
}

void recv_key() {
  struct fi_cq_msg_entry comp;
  fi_cq_sread(cq, &comp, 1, NULL, -1);
  assert(comp.flags & FI_RECV);
  memcpy(&recv_keys, recv_buff, sizeof(recv_keys));
}

void client_ping_pong_rma() {
  while (count-- > 0) {
    fi_read(ep, recv_buff, buff_size, fi_mr_desc(recv_mr), 0, recv_keys.addr, recv_keys.rkey, NULL);
    struct fi_cq_msg_entry comp;
    fi_cq_sread(cq, &comp, 1, NULL, -1);
  }
}

void client_ping_pong_msg() {
  memset(send_buff, '0', buff_size);
  fi_send(ep, send_buff, buff_size, fi_mr_desc(send_mr), 0, NULL);
  struct fi_cq_msg_entry comp;
  fi_cq_sread(cq, &comp, 1, NULL, -1);

  while (true) {
    fi_cq_sread(cq, &comp, 1, NULL, -1);
    fi_recv(ep, recv_buff, buff_size, fi_mr_desc(recv_mr), 0, NULL);
    if (--count <= 0) break;
    fi_send(ep, send_buff, buff_size, fi_mr_desc(send_mr), 0, NULL);
    fi_cq_sread(cq, &comp, 1, NULL, -1);
  }
}

void server_ping_pong_msg() {
  memset(send_buff, '0', buff_size);
  struct fi_cq_msg_entry comp;
  while (count--) {
    fi_cq_sread(cq, &comp, 1, NULL, -1);
    fi_recv(ep, recv_buff, buff_size, fi_mr_desc(recv_mr), 0, NULL);
    fi_send(ep, send_buff, buff_size, fi_mr_desc(send_mr), 0, NULL);
    fi_cq_sread(cq, &comp, 1, NULL, -1);
  }
}

void client_ping_pong_msg_one_side() {
  memset(send_buff, '0', buff_size);
  struct fi_cq_msg_entry comp;
  while (count--) {
    fi_send(ep, send_buff, buff_size, fi_mr_desc(send_mr), 0, NULL);
    fi_cq_sread(cq, &comp, 1, NULL, -1);
  }
}

void server_ping_pong_msg_one_side() {
  struct fi_cq_msg_entry comp;
  while (count--) {
    fi_cq_sread(cq, &comp, 1, NULL, -1);
    fi_recv(ep, recv_buff, buff_size, fi_mr_desc(recv_mr), 0, NULL);
  }
}

void shutdown() {
  fi_shutdown(ep, 0);
}

void join() {
  fi_eq_sread(eq, &event, &entry, sizeof(entry), -1, 0);

  assert(event == FI_SHUTDOWN);

  fi_close(&ep->fid);
  fi_close(&recv_mr->fid);
  fi_close(&send_mr->fid);
  free(recv_buff); 
  free(send_buff);
  fi_close(&cq->fid);
  fi_close(&eq->fid);
  fi_close(&domain->fid);
  fi_close(&fabric->fid);
  fi_freeinfo(hints);
  fi_freeinfo(info);
  
  printf("connection shutdown.\n");
}
