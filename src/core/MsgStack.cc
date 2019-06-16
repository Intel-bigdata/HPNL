#include "core/MsgStack.h"
#include "core/MsgConnection.h"

#include <iostream>

MsgStack::MsgStack(uint64_t flags_, int worker_num_, int buffer_num_, bool is_server_, const char* prov_name_) :
  flags(flags_),
  worker_num(worker_num_), buffer_num(buffer_num_), is_server(is_server_), prov_name(prov_name_),
  fabric(NULL), domain(NULL), hints(NULL), info(NULL), hints_tmp(NULL), info_tmp(NULL),
  peq(NULL), pep(NULL), waitset(NULL) {}

MsgStack::~MsgStack() {
  for (auto iter : conMap) {
    delete iter.second; 
  }
  conMap.erase(conMap.begin(), conMap.end());
  if (peq) {
    fi_close(&peq->fid);
    peq = nullptr;
  }
  for (int i = 0; i < worker_num; i++) {
    if (cqs[i]) {
      fi_close(&cqs[i]->fid);
      cqs[i] = nullptr;
    }
  }
  if (domain) {
    fi_close(&domain->fid);
    domain = nullptr;
  }
  if (fabric) {
    fi_close(&fabric->fid);
    fabric = nullptr;
  }
  if (hints) {
    fi_freeinfo(hints);
    hints = nullptr;
  }
  if (info) {
    fi_freeinfo(info);
    info = nullptr;
  }
  if (hints_tmp) {
    fi_freeinfo(hints_tmp);
    hints_tmp = nullptr;
  }
  if (is_server && info_tmp) {
    fi_freeinfo(info_tmp);
    info_tmp = nullptr;
  }
  prov_name = nullptr;
}

int MsgStack::init() {
  struct fi_eq_attr eq_attr = {
    .size = 0,
    .flags = 0,
    .wait_obj = FI_WAIT_UNSPEC,
    .signaling_vector = 0,
    .wait_set = NULL
  };

  if ((hints = fi_allocinfo()) == NULL) {
	perror("fi_allocinfo");
	goto free_hints;
  }
  hints->addr_format = FI_SOCKADDR_IN;
  hints->ep_attr->type = FI_EP_MSG;
  hints->domain_attr->mr_mode = FI_MR_BASIC;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT | FI_LOCAL_MR;
  hints->tx_attr->msg_order = FI_ORDER_SAS;
  hints->rx_attr->msg_order = FI_ORDER_SAS;
  if (prov_name != nullptr){
    hints->fabric_attr->prov_name = strdup(prov_name);
  }

  if (fi_getinfo(FI_VERSION(1, 5), NULL, NULL, flags, hints, &info)) {
    perror("fi_getinfo");
    goto free_info;
  }
  std::cout<<info->fabric_attr->prov_name<<"...."<<std::endl;
  if (fi_fabric(info->fabric_attr, &fabric, NULL)) {
	perror("fi_fabric");
    goto free_fabric;
  }
  if (fi_eq_open(fabric, &eq_attr, &peq, NULL)) {
    perror("fi_eq_open");
    goto free_eq;
  }
  if (fi_domain(fabric, info, &domain, NULL)) {
    perror("fi_domain");
    goto free_domain;
  }
 
  for (int i = 0; i < worker_num; i++) {
    struct fi_cq_attr cq_attr = {
      .size = 0,
      .flags = 0,
      .format = FI_CQ_FORMAT_MSG,
      .wait_obj = FI_WAIT_FD,
      .signaling_vector = 0,
      .wait_cond = FI_CQ_COND_NONE,
      .wait_set = NULL
    };

    if (fi_cq_open(domain, &cq_attr, &cqs[i], NULL)) {
      perror("fi_cq_open");
      goto free_cq;
    }
  }
  return 0;

free_cq:
  for (int i = 0; i < worker_num; i++) {
    if (cqs[i])
      fi_close(&cqs[i]->fid); 
  }
free_domain:
  if (domain) {
    fi_close(&domain->fid);
    domain = nullptr;
  }
free_eq:
  if (peq) {
    fi_close(&peq->fid);
    peq = nullptr;
  }
free_fabric:
  if (fabric) {
    fi_close(&fabric->fid);
    fabric = nullptr;
  }
free_info:
  if (info) {
    fi_freeinfo(info);
    info = nullptr;
  }
free_hints:
  if (hints) {
    fi_freeinfo(hints);
    hints = nullptr;
  }
  return -1;
}

void* MsgStack::bind(const char *ip_, const char *port_, BufMgr* rbuf, BufMgr* sbuf) {
  if ((hints_tmp = fi_allocinfo()) == NULL) {
	perror("fi_allocinfo");
  }
  hints_tmp->addr_format = FI_SOCKADDR_IN;
  hints_tmp->ep_attr->type = FI_EP_MSG;
  hints_tmp->domain_attr->mr_mode = FI_MR_BASIC;
  hints_tmp->caps = FI_MSG;
  hints_tmp->mode = FI_CONTEXT | FI_LOCAL_MR;
  hints_tmp->tx_attr->msg_order = FI_ORDER_SAS;
  hints_tmp->rx_attr->msg_order = FI_ORDER_SAS;
  if (prov_name != nullptr){
    hints_tmp->fabric_attr->prov_name = strdup(prov_name);
  }

  if (fi_getinfo(FI_VERSION(1, 5), ip_, port_, flags, hints_tmp, &info_tmp)) {
	perror("fi_getinfo");
  }

  if (fi_passive_ep(fabric, info_tmp, &pep, NULL)) {
    perror("fi_passive_ep");
    return NULL;
  }
  if (fi_pep_bind(pep, &peq->fid, 0)) {
    perror("fi_pep_bind");
    return NULL;
  }

  return peq;
}

int MsgStack::listen() {
  if (fi_listen(pep)) {
    perror("fi_listen");
    return -1; 
  }
  return 0;
}

fid_eq* MsgStack::connect(const char *ip_, const char *port_, int cq_index, long connect_id, BufMgr *recv_buf_mgr, BufMgr *send_buf_mgr) {
  if ((hints_tmp = fi_allocinfo()) == NULL) {
    perror("fi_allocinfo");
  }
  hints_tmp->addr_format = FI_SOCKADDR_IN;
  hints_tmp->ep_attr->type = FI_EP_MSG;
  hints_tmp->domain_attr->mr_mode = FI_MR_BASIC;
  hints_tmp->caps = FI_MSG;
  hints_tmp->mode = FI_CONTEXT | FI_LOCAL_MR;
  hints_tmp->tx_attr->msg_order = FI_ORDER_SAS;
  hints_tmp->rx_attr->msg_order = FI_ORDER_SAS;
  if (prov_name != nullptr){
    hints_tmp->fabric_attr->prov_name = strdup(prov_name);
  }

  if (fi_getinfo(FI_VERSION(1, 5), ip_, port_, flags, hints_tmp, &info_tmp)) {
    perror("fi_getinfo");
  }

  if(cq_index >= worker_num){
	perror("cq_index exceeds worker_num. "+cq_index);
	return NULL;
  }
  MsgConnection *con = new MsgConnection(this, fabric, info_tmp, domain, cqs[cq_index], waitset, recv_buf_mgr, send_buf_mgr, false, buffer_num, cq_index, connect_id);
  if (con->init())
    return NULL;
  if (int res = con->connect()) {
    if (res == EAGAIN) {
      // TODO: try again  
    } else {
      return NULL;  
    }
  }
  con->status = CONNECT_REQ;
  seq_num++;
  conMap.insert(std::pair<fid*, MsgConnection*>(con->get_fid(), con));
  return con->get_eq();
}

fid_eq* MsgStack::accept(void *info_, BufMgr *recv_buf_mgr, BufMgr *send_buf_mgr) {
  int cq_index = seq_num%worker_num;
  MsgConnection *con = new MsgConnection(this, fabric, (fi_info*)info_, domain, cqs[cq_index], waitset, recv_buf_mgr, send_buf_mgr, true, buffer_num, cq_index, 0);
  if (con->init())
    return NULL; 
  con->status = ACCEPT_REQ;
  seq_num++;
  conMap.insert(std::pair<fid*, MsgConnection*>(con->get_fid(), con));
  if (con->accept())
    return NULL;
  return con->get_eq();
}

uint64_t MsgStack::reg_rma_buffer(char* buffer, uint64_t buffer_size, int buffer_id) {
  Chunk *ck = new Chunk();
  ck->buffer = buffer;
  ck->capacity = buffer_size;
  ck->buffer_id = buffer_id;
  fid_mr *mr;
  if (fi_mr_reg(domain, ck->buffer, ck->capacity, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr, NULL)) {
    perror("fi_mr_reg");
    return -1;
  }
  ck->mr = mr;
  std::lock_guard<std::mutex> lk(mtx);
  chunkMap.insert(std::pair<int, Chunk*>(buffer_id, ck));
  return ((fid_mr*)ck->mr)->key;
}

void MsgStack::unreg_rma_buffer(int buffer_id) {
  Chunk *ck = get_rma_chunk(buffer_id);
  fi_close(&((fid_mr*)ck->mr)->fid);
}

Chunk* MsgStack::get_rma_chunk(int buffer_id) {
  std::lock_guard<std::mutex> lk(mtx);
  return chunkMap[buffer_id];
}

void MsgStack::shutdown() {
  //TODO: shutdown
}

void MsgStack::reap(void *con_id) {
  fid *id = (fid*)con_id;
  std::lock_guard<std::mutex> lk(conMtx);
  MsgConnection *con = get_connection(id);
  if(con){
	  con->status = DOWN;
	  conMap.erase(id);
  }
}

MsgConnection* MsgStack::get_connection(fid* id) {
  if (conMap.find(id) != conMap.end()) {
    return conMap[id]; 
  }
  return NULL;
}

fid_fabric* MsgStack::get_fabric() {
  return fabric;
}

fid_cq** MsgStack::get_cqs() {
  return cqs;
}

