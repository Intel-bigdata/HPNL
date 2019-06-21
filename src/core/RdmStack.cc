#include "core/RdmStack.h"
#include "core/RdmConnection.h"

#include <stdio.h>
#include <iostream>

RdmStack::RdmStack(int buffer_num_, bool is_server_) : buffer_num(buffer_num_), is_server(is_server_), domain(NULL), fabric(NULL), info(NULL), server_info(NULL), cq(NULL), initialized(false) {}

RdmStack::~RdmStack() {
  for (auto con : cons) {
    delete con;
    con = NULL;
  }
  if (cq)
    fi_close(&cq->fid);
  if (domain)
    fi_close(&domain->fid);
  if (fabric)
    fi_close(&fabric->fid);
  if (info)
    fi_freeinfo(info);
  if (is_server && server_info)
    fi_freeinfo(server_info);
}

int RdmStack::init() {
  fi_info* hints = fi_allocinfo();
  hints->ep_attr->type = FI_EP_RDM;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT;
#ifdef PSM2
  hints->fabric_attr->prov_name = strdup("psm2");
#elif VERBS
  hints->fabric_attr->prov_name = strdup("verbs");
#else
  hints->fabric_attr->prov_name = strdup("sockets");
#endif

  if (fi_getinfo(FI_VERSION(1, 5), NULL, NULL, is_server ? FI_SOURCE : 0, hints, &info))
    perror("fi_getinfo");
  fi_freeinfo(hints);

  if (fi_fabric(info->fabric_attr, &fabric, NULL))
    perror("fi_fabric");

  if (fi_domain(fabric, info, &domain, NULL))
    perror("fi_domain");

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
  initialized = true;
  return 0;
}

void* RdmStack::bind(const char* ip, const char* port, BufMgr* buf_mgr) {
  if (!initialized || !ip || !port || !buf_mgr)
    return NULL;
  fi_info* hints = fi_allocinfo();
  hints->ep_attr->type = FI_EP_RDM;
  hints->caps = FI_MSG;
  hints->mode = FI_CONTEXT;
#ifdef PSM2
  hints->fabric_attr->prov_name = strdup("psm2");
#elif VERBS
  hints->fabric_attr->prov_name = strdup("verbs");
#else
  hints->fabric_attr->prov_name = strdup("sockets");
#endif

  if (fi_getinfo(FI_VERSION(1, 5), ip, port, is_server ? FI_SOURCE : 0, hints, &server_info))
    perror("fi_getinfo");
  fi_freeinfo(hints);

  server_con = new RdmConnection(ip, port, server_info, domain, cq, buf_mgr, buffer_num, true);
  server_con->init();
  cons.push_back(server_con);
  return server_con;
}

RdmConnection* RdmStack::get_con(const char* ip, const char* port, BufMgr* buf_mgr) {
  if (!initialized || !ip || !port || !buf_mgr)
    return NULL;
  std::lock_guard<std::mutex> lk(mtx);
  if (buf_mgr->free_size() < buffer_num*2) {
    return NULL; 
  }
  RdmConnection *con = new RdmConnection(ip, port, NULL, domain, cq, buf_mgr, buffer_num, false);
  con->init();
  cons.push_back(con);
  return con;
}

fid_fabric* RdmStack::get_fabric() {
  return fabric;
}

fid_cq* RdmStack::get_cq() {
  return cq;
}
