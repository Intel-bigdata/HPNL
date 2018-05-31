#include "FIStack.h"

FIStack::FIStack(const char *addr, const char *port, uint64_t flags) : seq_num(0) {
  hints = fi_allocinfo();
  hints->addr_format = FI_SOCKADDR_IN;
  hints->ep_attr->type = FI_EP_MSG;
  hints->domain_attr->mr_mode = FI_MR_BASIC;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT | FI_LOCAL_MR;

  fi_getinfo(FI_VERSION(1, 5), addr, port, flags, hints, &info);
  fi_fabric(info->fabric_attr, &fabric, NULL);

  struct fi_eq_attr eq_attr = {
    .size = 0,
    .flags = 0,
    .wait_obj = FI_WAIT_UNSPEC,
    .signaling_vector = 0,
    .wait_set = NULL
  };
  assert(!fi_eq_open(fabric, &eq_attr, &peq, &peqHandle));

  fi_domain(fabric, info, &domain, NULL);
  
  for (int i = 0; i < WORKERS; i++) {
    struct fi_cq_attr cq_attr = {
      .size = 0,
      .flags = 0,
      .format = FI_CQ_FORMAT_MSG,
      .wait_obj = FI_WAIT_FD,
      .signaling_vector = 0,
      .wait_cond = FI_CQ_COND_NONE,
      .wait_set = NULL
    };

    fi_cq_open(domain, &cq_attr, &cqs[i], NULL);
  }

  recv_pool = new Mempool(domain, MEMPOOL_SIZE);
  send_pool = new Mempool(domain, MEMPOOL_SIZE);
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
  for (int i = 0; i < WORKERS; i++) {
    fi_close(&cqs[i]->fid); 
  }
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
  peqHandle.reset(new Handle(&peq->fid, EQ_EVENT, peq));
  return peqHandle;
}

void FIStack::listen() {
  fi_listen(pep);
}

HandlePtr FIStack::connect() {
  Mempool *rpool = new Mempool(recv_pool, CON_MEMPOOL_SIZE);
  Mempool *spool = new Mempool(send_pool, CON_MEMPOOL_SIZE);
  FIConnection *con = new FIConnection(fabric, info, domain, cqs[seq_num%WORKERS], waitset, rpool, spool, false);
  seq_num++;
  conMap.insert(std::pair<fid*, FIConnection*>(con->get_fid(), con));
  con->connect();
  return con->get_eqhandle();
}

HandlePtr FIStack::accept(void *info_) {
  Mempool *rpool = new Mempool(recv_pool, CON_MEMPOOL_SIZE);
  Mempool *spool = new Mempool(send_pool, CON_MEMPOOL_SIZE);
  FIConnection *con = new FIConnection(fabric, (fi_info*)info_, domain, cqs[seq_num%WORKERS], waitset, rpool, spool, true);
  seq_num++;
  conMap.insert(std::pair<fid*, FIConnection*>(con->get_fid(), con));
  con->accept();
  return con->get_eqhandle();
}

HandlePtr FIStack::connected(void *con_id) {
  return NULL;
}

void FIStack::shutdown() {

}

void FIStack::reap(void *con_id) {
  fid *id = (fid*)con_id;
  FIConnection *con = reinterpret_cast<FIConnection*>(get_connection(id));
  recv_pool->take(con->get_rpool(), CON_MEMPOOL_SIZE);
  send_pool->take(con->get_spool(), CON_MEMPOOL_SIZE);
  delete con;
  con = NULL;
  auto iter = conMap.find(id);
  assert(iter != conMap.end());
  conMap.erase(iter);
}

FIConnection* FIStack::get_connection(fid* id) {
  if (conMap.find(id) != conMap.end()) {
    return conMap[id]; 
  }
  return NULL;
}

fid_fabric* FIStack::get_fabric() {
  return fabric;
}

fid_cq** FIStack::get_cqs() {
  return cqs;
}
