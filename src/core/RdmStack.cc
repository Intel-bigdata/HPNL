#include "core/RdmStack.h"
#include "core/RdmConnection.h"

#include <stdio.h>
#include <iostream>

RdmStack::RdmStack(int buffer_num_, int recv_buffer_num_,int ctx_num_, bool is_server_, const char* prov_name_) :
buffer_num(buffer_num_), recv_buffer_num(recv_buffer_num_), ctx_num(ctx_num_), is_server(is_server_), prov_name(prov_name_) {}

RdmStack::~RdmStack() {
  for (auto item : conMap) {
    delete item.second;
  }
  conMap.erase(conMap.begin(), conMap.end());

  if(cq){
	  fi_close(&cq->fid);
	  cq = nullptr;
  }
  if(domain){
	  fi_close(&domain->fid);
	  domain = nullptr;
  }
  if(fabric){
	  fi_close(&fabric->fid);
	  fabric = nullptr;
  }
  if(info){
	  fi_freeinfo(info);
	  info = nullptr;
  }
  if (is_server){
	  if(server_info){
		  fi_freeinfo(server_info);
		  server_info = nullptr;
	  }
  }
}

int RdmStack::init() {
  fi_info* hints = fi_allocinfo();
  hints->ep_attr->type = FI_EP_RDM;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT;

//  hints->tx_attr->msg_order = FI_ORDER_SAS;
  hints->tx_attr->comp_order = FI_ORDER_NONE;
  hints->tx_attr->op_flags = FI_INJECT_COMPLETE | FI_COMPLETION;
  hints->rx_attr->op_flags = FI_COMPLETION;
  //hints->rx_attr->msg_order = FI_ORDER_SAS;
  hints->domain_attr->av_type         = FI_AV_MAP;
  hints->domain_attr->resource_mgmt   = FI_RM_ENABLED;
  hints->domain_attr->threading = FI_THREAD_SAFE;

  if (prov_name != nullptr){
    hints->fabric_attr->prov_name = strdup(prov_name);
  }

  if (fi_getinfo(FI_VERSION(1, 5), NULL, NULL, is_server ? FI_SOURCE : 0, hints, &info))
    perror("fi_getinfo");
  fi_freeinfo(hints);
  if (fi_fabric(info->fabric_attr, &fabric, NULL))
    perror("fi_fabric");
  if (fi_domain(fabric, info, &domain, NULL))
    perror("fi_domain");

  std::cout<<"provider: "<<info->fabric_attr->prov_name<<std::endl;
  
struct fi_cq_attr cq_attr = {
    .size = 0,
    .flags = 0,
    .format = FI_CQ_FORMAT_MSG,
    .wait_obj = FI_WAIT_FD,
    .signaling_vector = 0,
    .wait_cond = FI_CQ_COND_NONE,
    .wait_set = NULL
  };
  if (fi_cq_open(domain, &cq_attr, &cq, NULL)) {
    perror("fi_cq_open");
  }
  return 0;
}
void* RdmStack::bind(const char* ip, const char* port, BufMgr* rbuf_mgr, BufMgr* sbuf_mgr) {
  if (rbuf_mgr->free_size() < recv_buffer_num || sbuf_mgr->free_size() < buffer_num) {
    return NULL;
  }
  fi_info* hints = fi_dupinfo(info);
 
  if (fi_getinfo(FI_VERSION(1, 5), ip, port, is_server ? FI_SOURCE : 0, hints, &server_info))
    perror("fi_getinfo");
  fi_freeinfo(hints);

  long id = id_generator.fetch_add(1);
  server_con = new RdmConnection(ip, port, server_info, domain, cq, rbuf_mgr, sbuf_mgr, buffer_num, recv_buffer_num, ctx_num,
		  true, prov_name);
  server_con->set_id(id);
  server_con->init();
  conMap.insert(std::pair<long, RdmConnection*>(id, server_con));
  return server_con;
}

RdmConnection* RdmStack::get_con(const char* ip, const char* port, BufMgr* rbuf_mgr, BufMgr* sbuf_mgr) {
  std::lock_guard<std::mutex> lk(mtx);
  if (rbuf_mgr->free_size() < recv_buffer_num || sbuf_mgr->free_size() < buffer_num) {
    return NULL; 
  }
  long id = id_generator.fetch_add(1);
  RdmConnection *con = new RdmConnection(ip, port, NULL, domain, cq, rbuf_mgr, sbuf_mgr, buffer_num, recv_buffer_num, ctx_num,
		  false, prov_name);
  con->set_id(id);
  con->init();
  conMap.insert(std::pair<long, RdmConnection*>(id, con));
  return con;
}

RdmConnection* RdmStack::get_connection(long id){
  if (conMap.find(id) != conMap.end()) {
	return conMap[id];
  }
  return NULL;
}

void RdmStack::reap(long id) {
  std::lock_guard<std::mutex> lk(conMtx);
  RdmConnection *con = get_connection(id);
  if(con){
	  conMap.erase(id);
  }
}

fid_fabric* RdmStack::get_fabric() {
  return fabric;
}

fid_cq* RdmStack::get_cq() {
  return cq;
}
