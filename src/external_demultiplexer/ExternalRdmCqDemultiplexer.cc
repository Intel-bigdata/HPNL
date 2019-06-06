#include <assert.h>

#include <iostream>
#include <chrono>

#include "external_demultiplexer/ExternalRdmCqDemultiplexer.h"
#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "demultiplexer/EventType.h"

ExternalRdmCqDemultiplexer::ExternalRdmCqDemultiplexer(RdmStack *stack_) : stack(stack_), start(0), end(0) {}

ExternalRdmCqDemultiplexer::~ExternalRdmCqDemultiplexer() {
  close(epfd);
}

int ExternalRdmCqDemultiplexer::init() {
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

int ExternalRdmCqDemultiplexer::wait_event(Chunk** ck, int *block_buffer_size) {
  struct fid *fids[1];
  fids[0] = &cq->fid;
  int ret = 0;
  if (end - start >= 2000000) {
    if (fi_trywait(fabric, fids, 1) == FI_SUCCESS) {
      int epoll_ret = epoll_wait(epfd, &event, 1, 2000);
      if (event.data.ptr != (void*)&cq->fid) {
        std::cout << "Epoll wait error." << std::endl;
      }
      if (epoll_ret <= 0) {
        return 0;
      }
    }
    start = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  }
  fi_cq_msg_entry entry;
  ret = fi_cq_read(cq, &entry, 1);
  if (ret == -FI_EAVAIL) {
    fi_cq_err_entry err_entry;
    fi_cq_readerr(cq, &err_entry, entry.flags); 
    end = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
    std::cout << "fi_cq_read ERROR." << std::endl;
    return -1;
  } else if (ret == -FI_EAGAIN) {
    end = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
    return 0;
  } else {
    end = start;
    if (entry.flags & FI_RECV) {
      fi_context2 *ctx = (fi_context2*)entry.op_context;
      *ck = (Chunk*)ctx->internal[4];
      *block_buffer_size = entry.len;
      return RECV_EVENT;
    } else if (entry.flags & FI_SEND) {
      fi_context2 *ctx = (fi_context2*)entry.op_context;
      if (ctx->internal[4] == NULL) {
        std::free(ctx);
      } else {
        *ck = (Chunk*)ctx->internal[4];
      }
      return SEND_EVENT;
    } else if (entry.flags & FI_READ) {
      return READ_EVENT;
    } else if (entry.flags & FI_WRITE) {
      return WRITE_EVENT;
    } else {
      return 0;
    }
  }
  return 0;
}
