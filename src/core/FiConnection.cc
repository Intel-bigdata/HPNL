#include <netinet/in.h>
#include <arpa/inet.h>

#include "core/FiStack.h"
#include "core/FiConnection.h"

FiConnection::FiConnection(FiStack *stack_, fid_fabric *fabric_, 
    fi_info *info_, fid_domain *domain_, fid_cq* cq_, 
    fid_wait *waitset_, BufMgr *recv_buf_mgr_, 
    BufMgr *send_buf_mgr_, bool is_server_, int buffer_num_, int cq_index_) : 
  stack(stack_), fabric(fabric_), info(info_), domain(domain_), ep(NULL),
  conCq(cq_), conEq(NULL), recv_buf_mgr(recv_buf_mgr_), send_buf_mgr(send_buf_mgr_), 
  waitset(waitset_), is_server(is_server_), buffer_num(buffer_num_), cq_index(cq_index_),
  recv_callback(NULL), send_callback(NULL), shutdown_callback(NULL) {}

FiConnection::~FiConnection() {
  for (auto buffer: send_buffers_map) {
    Chunk *ck = buffer.second;
    fi_close(&((fid_mr*)ck->mr)->fid);
    send_buf_mgr->put(ck->buffer_id, ck);
  }
  while (recv_buffers.size() > 0) {
    Chunk *ck = recv_buffers.back();
    fi_close(&((fid_mr*)ck->mr)->fid);
    recv_buffers.pop_back();
    recv_buf_mgr->put(ck->buffer_id, ck);
  }
  if (ep) {
    shutdown();
    fi_close(&ep->fid);
    ep = nullptr;
  }
  if (conEq) {
    fi_close(&conEq->fid);
    conEq = nullptr;
  }
  if (info) {
    fi_freeinfo(info);
    info = nullptr;
  }
}

int FiConnection::init() {
  int size = 0;
  struct fi_eq_attr eq_attr = {
    .size = 0,
    .flags = 0,
    .wait_obj = FI_WAIT_UNSPEC,
    .signaling_vector = 0,
    .wait_set = NULL
  };

  if (fi_endpoint(domain, info, &ep, NULL)) {
    perror("fi_endpoint");
    goto free_ep;
  }

  if (fi_eq_open(fabric, &eq_attr, &conEq, NULL)) {
    perror("fi_eq_open");
    goto free_eq;
  }

  if (fi_ep_bind(ep, &conEq->fid, 0)) {
    perror("fi_ep_bind");
    goto free_eq;
  }

  if (fi_ep_bind(ep, &conCq->fid, FI_TRANSMIT | FI_RECV)) {
    perror("fi_ep_bind");
    goto free_eq;
  }
  
  fi_enable(ep);
  while (size < buffer_num*2) {
    fid_mr *mr;
    Chunk *ck = recv_buf_mgr->get();
    if (fi_mr_reg(domain, ck->buffer, ck->capacity, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr, NULL)) {
      perror("fi_mr_reg");
      goto free_recv_buf;
    }
    ck->con = this;
    ck->mr = mr;
    if (fi_recv(ep, ck->buffer, ck->capacity, fi_mr_desc(mr), 0, ck)) {
      perror("fi_recv");
      goto free_recv_buf;
    }
    mr = NULL;
    recv_buffers.push_back(ck);
    size++;
  }
  size = 0;
  while (size < buffer_num) {
    fid_mr *mr;
    Chunk *ck = send_buf_mgr->get();
    if (fi_mr_reg(domain, ck->buffer, ck->capacity, FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV, 0, 0, 0, &mr, NULL)) {
      perror("fi_mr_reg");
      goto free_send_buf;
    }
    ck->con = this;
    ck->mr = mr;
    mr = NULL;
    send_buffers.push_back(ck);
    send_buffers_map.insert(std::pair<int, Chunk*>(ck->buffer_id, ck));
    size++;
  }

  return 0;

free_send_buf:
  for (auto buffer: send_buffers_map) {
    Chunk *ck = buffer.second;
    fi_close(&((fid_mr*)ck->mr)->fid);
    send_buf_mgr->put(ck->buffer_id, ck);
  }
free_recv_buf:
  while (recv_buffers.size() > 0) {
    Chunk *ck = recv_buffers.back();
    fi_close(&((fid_mr*)ck->mr)->fid);
    recv_buffers.pop_back();
    recv_buf_mgr->put(ck->buffer_id, ck);
  }
free_eq:
  if (conEq) {
    fi_close(&conEq->fid);
    conEq = nullptr;
  }
free_ep:
  if (ep) {
    fi_close(&ep->fid);
    ep = nullptr;
  }

  return -1;
}

int FiConnection::send(const char *buffer, int buffer_size, long seq) {
  Chunk *ck = send_buffers.back();
  send_buffers.pop_back();
  memcpy(ck->buffer, buffer, buffer_size);
  if (fi_send(ep, ck->buffer, buffer_size, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int FiConnection::send(int buffer_size, int buffer_id) {
  Chunk *ck = send_buffers_map[buffer_id];
  if (fi_send(ep, ck->buffer, buffer_size, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

void FiConnection::recv(char *buffer, int buffer_size) {
  // TODO: buffer filter
}

int FiConnection::read(int buffer_id, int local_offset, uint64_t len, uint64_t remote_addr, uint64_t remote_key) {
  Chunk *ck = stack->get_rma_chunk(buffer_id);
  ck->con = this;
  return fi_read(ep, (char*)ck->buffer+local_offset, len, fi_mr_desc((fid_mr*)ck->mr), 0, remote_addr, remote_key, ck);
}

int FiConnection::connect() {
  int res = fi_connect(ep, info->dest_addr, NULL, 0);
  if (res) {
    if (res == EAGAIN) {
      return EAGAIN;
    } else {
      perror("fi_connect");
      return -1;
    }
  }
  return 0;
}

int FiConnection::accept() {
  if (fi_accept(ep, NULL, 0)) {
    perror("fi_accept");
    return -1;
  }
  return 0;
}

void FiConnection::shutdown() {
  fi_shutdown(ep, 0);
}

void FiConnection::init_addr() {
  if (info->dest_addr != NULL) {
    struct sockaddr_in *dest_addr_in = (struct sockaddr_in*)info->dest_addr;
    dest_port = dest_addr_in->sin_port;
    char *addr = inet_ntoa(dest_addr_in->sin_addr);
    strcpy(dest_addr, addr);
  }

  if (info->src_addr != NULL) {
    struct sockaddr_in *src_addr_in = (struct sockaddr_in*)info->src_addr;
    src_port = src_addr_in->sin_port;
    char *addr = inet_ntoa(src_addr_in->sin_addr);
    strcpy(src_addr, addr);
  }
}

void FiConnection::get_addr(char** dest_addr_, size_t* dest_port_, char** src_addr_, size_t* src_port_) {
  *dest_addr_ = dest_addr;
  *dest_port_ = dest_port;

  *src_addr_ = src_addr;
  *src_port_ = src_port;
}

int FiConnection::get_cq_index() {
  return cq_index;
}

void FiConnection::take_back_chunk(Chunk *ck) {
  send_buffers.push_back(ck);
}

std::vector<Chunk*> FiConnection::get_send_buffer() {
  return send_buffers;
}

void FiConnection::set_recv_callback(Callback *callback) {
  recv_callback = callback;
}

void FiConnection::set_send_callback(Callback *callback) {
  send_callback = callback;
}

void FiConnection::set_read_callback(Callback *callback) {
  read_callback = callback;
}

void FiConnection::set_shutdown_callback(Callback *callback) {
  shutdown_callback = callback;
}

Callback* FiConnection::get_recv_callback() {
  return recv_callback;
}

Callback* FiConnection::get_send_callback() {
  return send_callback;
}

Callback* FiConnection::get_read_callback() {
  return read_callback;
}

Callback* FiConnection::get_shutdown_callback() {
  return shutdown_callback;
}

fid* FiConnection::get_fid() {
  return &conEq->fid;
}

int FiConnection::activate_chunk(Chunk *ck) {
  ck->con = this;
  if (fi_recv(ep, ck->buffer, ck->capacity, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    perror("fi_recv");
    return -1; 
  }
  return 0;
}

fid_eq* FiConnection::get_eq() {
  return conEq;
}
