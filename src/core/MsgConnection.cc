#include <netinet/in.h>
#include <arpa/inet.h>

#include "core/MsgStack.h"
#include "core/MsgConnection.h"

MsgConnection::MsgConnection(MsgStack *stack_, fid_fabric *fabric_,
    fi_info *info_, fid_domain *domain_, fid_cq* cq_, 
    fid_wait *waitset_, BufMgr *recv_buf_mgr_, 
    BufMgr *send_buf_mgr_, bool is_server_, int buffer_num_, int cq_index_, long connect_id_) :
  stack(stack_), fabric(fabric_), info(info_), domain(domain_), ep(NULL),
  conCq(cq_), conEq(NULL), recv_buf_mgr(recv_buf_mgr_), send_buf_mgr(send_buf_mgr_), 
  waitset(waitset_), is_server(is_server_), buffer_num(buffer_num_), cq_index(cq_index_),
  connect_id(connect_id_), read_callback(NULL), send_callback(NULL), shutdown_callback(NULL) {}

MsgConnection::~MsgConnection() {
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

int MsgConnection::init() {
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

int MsgConnection::sendBuf(const char *buffer, int block_buffer_size) {
  Chunk *ck = send_buffers.back();
  send_buffers.pop_back();
  memcpy(ck->buffer, buffer, block_buffer_size);
  if (fi_send(ep, ck->buffer, block_buffer_size, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int MsgConnection::send(int block_buffer_size, int buffer_id) {
  Chunk *ck = send_buffers_map[buffer_id];
  if (fi_send(ep, ck->buffer, block_buffer_size, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    perror("fi_send");
    return -1;
  }
  return 0;
}

int MsgConnection::read(int buffer_id, int local_offset, uint64_t len, uint64_t remote_addr, uint64_t remote_key) {
  Chunk *ck = stack->get_rma_chunk(buffer_id);
  ck->con = this;
  return fi_read(ep, (char*)ck->buffer+local_offset, len, fi_mr_desc((fid_mr*)ck->mr), 0, remote_addr, remote_key, ck);
}

int MsgConnection::connect() {
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

int MsgConnection::accept() {
  if (fi_accept(ep, NULL, 0)) {
    perror("fi_accept");
    return -1;
  }
  return 0;
}

void MsgConnection::shutdown() {
  fi_shutdown(ep, 0);
}

void MsgConnection::init_addr() {
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

void MsgConnection::get_addr(char** dest_addr_, size_t* dest_port_, char** src_addr_, size_t* src_port_) {
  *dest_addr_ = dest_addr;
  *dest_port_ = dest_port;

  *src_addr_ = src_addr;
  *src_port_ = src_port;
}

void MsgConnection::reclaim_chunk(Chunk *ck) {
  send_buffers.push_back(ck);
}

std::vector<Chunk*> MsgConnection::get_send_buffer() {
  return send_buffers;
}

std::vector<Chunk*> MsgConnection::get_recv_buffer(){
  return recv_buffers;
}

void MsgConnection::set_recv_callback(Callback *callback) {
  read_callback = callback;
}

void MsgConnection::set_send_callback(Callback *callback) {
  send_callback = callback;
}

void MsgConnection::set_shutdown_callback(Callback *callback) {
  shutdown_callback = callback;
}

Callback* MsgConnection::get_read_callback() {
  return read_callback;
}

Callback* MsgConnection::get_send_callback() {
  return send_callback;
}

Callback* MsgConnection::get_shutdown_callback() {
  return shutdown_callback;
}

fid* MsgConnection::get_fid() {
  return &conEq->fid;
}

int MsgConnection::get_cq_index(){
  return cq_index;
}

void MsgConnection::set_cq_index(int index){
  cq_index = index;
}

long MsgConnection::get_id(){
  return connect_id;
}

int MsgConnection::activate_chunk(Chunk *ck) {
  ck->con = this;
  if (fi_recv(ep, ck->buffer, ck->capacity, fi_mr_desc((fid_mr*)ck->mr), 0, ck)) {
    perror("fi_recv");
    return -1; 
  }
  return 0;
}

int MsgConnection::activate_chunk(int bufferId) {
  Chunk *ck = recv_buf_mgr->get(bufferId);
  return activate_chunk(ck);
}

fid_eq* MsgConnection::get_eq() {
  return conEq;
}
