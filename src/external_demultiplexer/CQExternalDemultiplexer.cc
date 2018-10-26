#include "HPNL/CQExternalDemultiplexer.h"

CQExternalDemultiplexer::CQExternalDemultiplexer(FIStack *stack, fid_cq *cq_) : cq(cq_) {
  fabric = stack->get_fabric();
  epfd = epoll_create1(0);
  memset((void*)&event, 0, sizeof event);
  int ret = fi_control(&cq->fid, FI_GETWAIT, (void*)&fd);
  if (ret) {
    std::cout << "fi_controll error." << std::endl; 
  }
  event.events = EPOLLIN;
  event.data.ptr = &cq->fid;
  ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
  if (ret) {
    std::cout << "epoll add error." << std::endl; 
  }
  start = 0;
  end = 0;
}

CQExternalDemultiplexer::~CQExternalDemultiplexer() {
  close(epfd);
}

int CQExternalDemultiplexer::wait_event(fid_eq** eq, int* rdma_buffer_id, int* block_buffer_size) {
  struct fid *fids[1];
  fids[0] = &cq->fid;
  int ret = 0;
  if (end - start >= 20000) {
    if (fi_trywait(fabric, fids, 1) == FI_SUCCESS) {
      int epoll_ret = epoll_wait(epfd, &event, 1, 2000);
      if (epoll_ret < 0) {
        std::cout << "error" << std::endl;
        return epoll_ret;
      }
      if (event.data.ptr != (void*)&cq->fid) {
        std::cout << "got error event" << std::endl;
      }
    }
    start = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  }
  fi_cq_msg_entry entry;
  while (ret > 0) {
    ret = fi_cq_read(cq, &entry, 1);
    if (ret == -FI_EAVAIL) {
      fi_cq_err_entry err_entry;
      fi_cq_readerr(cq, &err_entry, entry.flags); 
      std::cout << "error" << std::endl;
      end = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
      return -1;
    } else if (ret == -FI_EAGAIN) {
      end = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
      return 0;
    } else {
      end = start;
      Chunk *ck = (Chunk*)entry.op_context;
      *rdma_buffer_id = ck->rdma_buffer_id;
      FIConnection *con = (FIConnection*)ck->con;
      if (!con) {
        return 0;
      }
      fid_eq *eq_tmp = (fid_eq*)con->get_eqhandle()->get_ctx();
      *eq = eq_tmp;
      if (entry.flags & FI_RECV) {
        std::unique_lock<std::mutex> l(con->con_mtx);
        con->con_cv.wait(l, [con] { return con->status >= CONNECTED; });
        l.unlock();
        con->recv((char*)ck->buffer, entry.len);
        *block_buffer_size = entry.len;
        return RECV_EVENT;
      } else if (entry.flags & FI_SEND) {
        return SEND_EVENT;
      } else if (entry.flags & FI_READ) {
        return READ_EVENT;
      } else if (entry.flags & FI_WRITE) {
        return WRITE_EVENT;
      } else {
        return 0;
      }
    }
  }
  return 0;
}
