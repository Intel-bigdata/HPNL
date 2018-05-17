#include "FIStack.h"

FIStack::FIStack(const char *addr, uint64_t flags) : seq_num(0) {
  hints = fi_allocinfo();
  hints->addr_format = FI_SOCKADDR_IN;
  hints->ep_attr->type = FI_EP_MSG;
  hints->domain_attr->mr_mode = FI_MR_BASIC;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT;

  fi_getinfo(FI_VERSION(1, 5), addr, "12345", flags, hints, &info);

  fi_fabric(info->fabric_attr, &fabric, NULL);

  struct fi_eq_attr eq_attr = {
    .size = 0,
    .flags = 0,
    .wait_obj = FI_WAIT_UNSPEC,
    .signaling_vector = 0,
    .wait_set = NULL
  };

  assert(!fi_eq_open(fabric, &eq_attr, &peq, &pcmHandle));

  fi_domain(fabric, info, &domain, NULL);
  recv_pool = new Mempool(domain, 1024);
  send_pool = new Mempool(domain, 1024);
  recv_pool->register_memory();
  send_pool->register_memory();
}

FIStack::~FIStack() {
  for (auto iter : conMap) {
    delete iter.second; 
  }
  conMap.erase(conMap.begin(), conMap.end());
  fi_close(&pep->fid);
  fi_close(&peq->fid);
  fi_close(&domain->fid);
  fi_close(&fabric->fid);
  fi_freeinfo(hints);
  fi_freeinfo(info);

  recv_pool->release_memory();
  send_pool->release_memory();
  delete recv_pool;
  delete send_pool;
}

HandlePtr FIStack::bind() {
  fi_passive_ep(fabric, info, &pep, NULL);  
  
  fi_pep_bind(pep, &peq->fid, 0);

  pcmHandle.reset(new Handle(&peq->fid, EQ_EVENT, peq));
  return pcmHandle;
}

void FIStack::listen() {
  fi_listen(pep);
}

HandlePtr FIStack::connect() {
  Mempool *rpool = new Mempool(recv_pool, 32);
  Mempool *spool = new Mempool(send_pool, 32);
  FIConnection *con = new FIConnection(fabric, info, domain, rpool, spool, false);
  conMap.insert(std::pair<fid*, FIConnection*>(con->get_fid(), con));
  con->connect();
  return con->get_cmhandle();
}

HandlePtr FIStack::accept(void *info_) {
  Mempool *rpool = new Mempool(recv_pool, 32);
  Mempool *spool = new Mempool(send_pool, 32);
  FIConnection *con = new FIConnection(fabric, (fi_info*)info_, domain, rpool, spool, true);
  conMap.insert(std::pair<fid*, FIConnection*>(con->get_fid(), con));
  con->accept();
  return con->get_cmhandle();
}

HandlePtr FIStack::connected(void *con_id) {
  fid *id = (fid*)con_id;
  return ((FIConnection*)get_connection(id))->connected();
}

void FIStack::shutdown() {

}

void FIStack::reap(void *con_id) {
  fid *id = (fid*)con_id;
  FIConnection *con = (FIConnection*)get_connection(id);
  recv_pool->take(con->get_rpool(), 32);
  send_pool->take(con->get_spool(), 32);
  delete con;
  auto iter = conMap.find(id);
  assert(iter != conMap.end());
  conMap.erase(iter);
}

Connection* FIStack::get_connection(fid* id) {
  if (conMap.find(id) != conMap.end()) {
    return conMap[id]; 
  }
  return NULL;
}

void* FIStack::get_domain() {
  return domain;
}
