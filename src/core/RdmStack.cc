#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "core/Constant.h"
#include <stdlib.h>

#include <stdio.h>
#include <iostream>

RdmStack::RdmStack(int buffer_num_, int recv_buffer_num_,int ctx_num_, int endpoint_num_, bool is_server_, const char* prov_name_) :
buffer_num(buffer_num_), recv_buffer_num(recv_buffer_num_), ctx_num(ctx_num_), endpoint_num(endpoint_num_),
is_server(is_server_), prov_name(prov_name_) {
	is_setup = false;
	server_con = nullptr;
}

RdmStack::~RdmStack() {
  for (auto item : conMap) {
    delete item.second;
  }
  conMap.erase(conMap.begin(), conMap.end());

  int i;
  for(i=0; i<endpoint_num; i++){
	  if(cqs){
		  fi_close(&cqs[i]->fid);
	  }
	  if(tx){
		  fi_close(&tx[i]->fid);
	  }
	  if(rx){
		  fi_close(&rx[i]->fid);
	  }
  }
  if(cqs){
 	  delete[] cqs;
  }
  if(tx){
	  delete[] tx;
  }
  if(rx){
	  delete[] rx;
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
  if (connect_info){
	fi_freeinfo(connect_info);
	connect_info = nullptr;
  }
  if(local_name){
	  free(local_name);
  }
}

int RdmStack::init() {
  fi_info* hints = fi_allocinfo();
  hints->ep_attr->type = FI_EP_RDM;
  hints->caps = FI_TAGGED | FI_DIRECTED_RECV;
  hints->mode = FI_CONTEXT;

//  hints->tx_attr->msg_order = FI_ORDER_SAS;
  hints->tx_attr->comp_order = FI_ORDER_NONE;
  hints->tx_attr->op_flags = FI_COMPLETION;
  hints->rx_attr->op_flags = FI_COMPLETION;
  hints->ep_attr->tx_ctx_cnt = endpoint_num;
  hints->ep_attr->rx_ctx_cnt = endpoint_num;
  //hints->rx_attr->msg_order = FI_ORDER_SAS;
  hints->domain_attr->av_type         = FI_AV_UNSPEC;
  hints->domain_attr->resource_mgmt   = FI_RM_ENABLED;
  hints->domain_attr->threading = FI_THREAD_UNSPEC;

  if (prov_name != nullptr){
    hints->fabric_attr->prov_name = strdup(prov_name);
  }

  if (fi_getinfo(FI_VERSION(1, 8), NULL, NULL, is_server ? FI_SOURCE : 0, hints, &info)){
    perror("fi_getinfo");
  }
  std::cout<<"provider: "<<info->fabric_attr->prov_name<<std::endl;

  fi_freeinfo(hints);
  if (fi_fabric(info->fabric_attr, &fabric, NULL)){
    perror("fi_fabric");
  }
  if (fi_domain(fabric, info, &domain, NULL)){
    perror("fi_domain");
  }

  //CQs
  struct fi_cq_attr cq_attr = {
    .size = 0,
    .flags = 0,
    .format = FI_CQ_FORMAT_TAGGED,
    .wait_obj = FI_WAIT_FD,
    .signaling_vector = 0,
    .wait_cond = FI_CQ_COND_NONE,
    .wait_set = NULL
  };
  cqs = new fid_cq*[endpoint_num];
  int size = 0;
  while(size < endpoint_num){//cqs[0] for connection setup
	  if (fi_cq_open(domain, &cq_attr, &cqs[size], NULL)) {
		perror("fi_cq_open");
	  }
	  size++;
  }

  //AV
  fi_av_attr	av_attr;
  memset(&av_attr, 0, sizeof(av_attr));
  av_attr.type = FI_AV_UNSPEC;
  av_attr.rx_ctx_bits = RECV_CTX_BITS;
  if (fi_av_open(domain, &av_attr, &av, NULL)){
	  perror("fi_av_open");
  }

  return 0;
}

void RdmStack::setup_endpoint(const char* ip, const char* port){
  fi_info* hints = fi_dupinfo(info);
  if (fi_getinfo(FI_VERSION(1, 8), ip, port, is_server ? FI_SOURCE:0, hints,
		  &connect_info)){
	perror("fi_getinfo");
  }
  fi_freeinfo(hints);

  std::cout<<"connect info: "<<connect_info->ep_attr->tx_ctx_cnt<<std::endl;
  std::cout<<"connect info: "<<connect_info->ep_attr->rx_ctx_cnt<<std::endl;
  //endpoint
  if (fi_scalable_ep(domain, connect_info, &ep, NULL)) {
	perror("fi_endpoint");
  }

  fi_scalable_ep_bind(ep, &av->fid, 0);
  fi_enable(ep);

  //contexts
  tx = new fid_ep*[endpoint_num];
  int i;
  for(i=0; i<endpoint_num; i++){
	  fi_tx_context(ep, i, NULL, &tx[i], NULL);
	  fi_ep_bind(tx[i], &(cqs[i]->fid), FI_SEND);
	  fi_enable(tx[i]);
  }
  rx = new fid_ep*[endpoint_num];
  for(i=0; i<endpoint_num; i++){
	  fi_rx_context(ep, i, NULL, &rx[i], NULL);
	  fi_ep_bind(rx[i], &(cqs[i]->fid), FI_RECV);
	  fi_enable(rx[i]);
  }

  local_name = (char *)malloc(64);
  local_name_len = 64;
  fi_getname(&ep->fid, local_name, &(local_name_len));
  printf("local name len: %d\n", local_name_len);
	for(i=0; i<local_name_len; i++)
		printf("local name: %d\n", local_name[i]);
}

void* RdmStack::bind(const char* ip, const char* port, BufMgr* rbuf_mgr, BufMgr* sbuf_mgr) {
  std::lock_guard<std::mutex> lk(mtx);
  if(!is_setup){
	  setup_endpoint(ip, port);
	  is_setup = true;
  }
  if(server_con){
  	  return server_con;
  }

  if (rbuf_mgr->free_size() < recv_buffer_num || sbuf_mgr->free_size() < buffer_num) {
    return NULL;
  }

  long id = id_generator.fetch_add(1);
  server_con = new RdmConnection(connect_info, av,
		  FI_ADDR_UNSPEC, cqs[0], tx[0], rx[0], rbuf_mgr, sbuf_mgr, true);
  server_con->set_id(id);
  server_con->set_local_name(local_name, local_name_len);
  server_con->init(buffer_num, recv_buffer_num, ctx_num, 0, 0);
  conMap.insert(std::pair<long, RdmConnection*>(id, server_con));
  return server_con;
}

RdmConnection* RdmStack::get_con(const char* ip, const char* port, uint64_t dest_provider_addr,
		int cq_index, int send_ctx_id, BufMgr* rbuf_mgr, BufMgr* sbuf_mgr) {
  std::lock_guard<std::mutex> lk(mtx);
  if(!is_setup){
	  setup_endpoint(ip, port);
	  is_setup = true;
  }
  if (rbuf_mgr->free_size() < recv_buffer_num || sbuf_mgr->free_size() < buffer_num) {
      return NULL;
  }

  long id = id_generator.fetch_add(1);
  if(is_server){
	  assert(dest_provider_addr >= 0);
	  assert(cq_index > 0);
  }else{
	  assert(cq_index == 0);
  }
  RdmConnection *con = new RdmConnection(connect_info, av,
		  is_server ? dest_provider_addr:FI_ADDR_UNSPEC,
				  cqs[cq_index], tx[cq_index], rx[cq_index], rbuf_mgr, sbuf_mgr, false);
  con->set_id(id);
  con->set_accepted_connection(is_server);
  con->set_local_name(local_name, local_name_len);
  con->init(buffer_num, recv_buffer_num, ctx_num, send_ctx_id, 0);
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

fid_cq ** RdmStack::get_cqs() {
  return cqs;
}
