#include "core/RdmStack.h"
#include "core/RdmConnection.h"

#include <stdio.h>

RdmStack::RdmStack(int buffer_num_, bool is_server_) : buffer_num(buffer_num_), is_server(is_server_) {}

RdmStack::~RdmStack() {}

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

  return 0;
}

void* RdmStack::bind(const char* ip, const char* port, BufMgr* rbuf_mgr, BufMgr* sbuf_mgr) {
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

  server_con = new RdmConnection(ip, port, server_info, domain, cq, rbuf_mgr, sbuf_mgr, buffer_num, true);
  server_con->init();

  return server_con;
}

RdmConnection* RdmStack::get_con(const char* ip, const char* port, BufMgr* rbuf_mgr, BufMgr* sbuf_mgr) {
  RdmConnection *con = new RdmConnection(ip, port, NULL, domain, cq, rbuf_mgr, sbuf_mgr, buffer_num, false);
  con->init();
  conMap.insert(std::pair<uint64_t, RdmConnection*>(id++, con));
  return con;
}

fid_fabric* RdmStack::get_fabric() {
  return fabric;
}

fid_cq* RdmStack::get_cq() {
  return cq;
}
