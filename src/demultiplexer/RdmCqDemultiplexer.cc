#include <assert.h>

#include <iostream>
#include <chrono>

#include "demultiplexer/RdmCqDemultiplexer.h"
#include "core/RdmStack.h"
#include "core/RdmConnection.h"

RdmCqDemultiplexer::RdmCqDemultiplexer(RdmStack *stack_) : stack(stack_) {}

RdmCqDemultiplexer::~RdmCqDemultiplexer() {
  close(epfd);
}

int RdmCqDemultiplexer::init() {
  fabric = stack->get_fabric();
  cq = stack->get_cq();
  epfd = epoll_create1(0);
  memset((void*)&event, 0, sizeof event);
  int ret = fi_control(&cq->fid, FI_GETWAIT, (void*)&fd);
  if (ret) {
    std::cout << "fi_controll error." << std::endl; 
    return -1;
  }
  event.events = EPOLLIN;
  event.data.ptr = &cq->fid;
  ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
  if (ret) {
    std::cout << "epoll add error." << std::endl; 
    return -1;
  }
  return 0;
}

int RdmCqDemultiplexer::wait_event() {
  struct fid *fids[1];
  fids[0] = &cq->fid;
  int ret = 0;
  if (fi_trywait(fabric, fids, 1) == FI_SUCCESS) {
    int epoll_ret = epoll_wait(epfd, &event, 1, 200);
    if (event.data.ptr != (void*)&cq->fid) {
      std::cout << "Epoll wait error." << std::endl;
    }
    if (epoll_ret <= 0) {
      return 0;
    }
  }
  uint64_t start, end = 0;
  start = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  end = start;
  do {
    fi_cq_msg_entry entry;
    ret = fi_cq_read(cq, &entry, 1);
    if (ret == -FI_EAVAIL) {
      fi_cq_err_entry err_entry;
      fi_cq_readerr(cq, &err_entry, entry.flags); 
      std::cout << "fi_cq_read ERROR." << std::endl;
      break;
    } else if (ret == -FI_EAGAIN) {
    } else {
      if (entry.flags & FI_RECV) {
        fi_context2 *ctx = (fi_context2*)entry.op_context;
        Chunk *ck = (Chunk*)ctx->internal[4];
        RdmConnection *con = (RdmConnection*)ck->con;
        if (con->get_recv_callback()) {
          (*con->get_recv_callback())(&ck->buffer_id, &entry.len);
        }
        con->activate_recv_chunk(ck);
      } else if (entry.flags & FI_SEND) {
        fi_context2 *ctx = (fi_context2*)entry.op_context;
        Chunk *ck = (Chunk*)ctx->internal[4];
        RdmConnection *con = (RdmConnection*)ck->con;
        con->activate_send_chunk(ck);
      } else if (entry.flags & FI_READ) {
      } else if (entry.flags & FI_WRITE) {
      } else {
      }
      start = end;
    }
    end = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  } while (end-start <= 100000);
  return 0;
}
