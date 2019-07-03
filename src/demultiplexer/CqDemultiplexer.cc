#include <assert.h>

#include <iostream>

#include "demultiplexer/CqDemultiplexer.h"
#include "core/MsgStack.h"
#include "core/MsgConnection.h"

CqDemultiplexer::CqDemultiplexer(MsgStack *stack_, int work_num_) : stack(stack_), cq(nullptr), work_num(work_num_) {}

CqDemultiplexer::~CqDemultiplexer() {
  #ifdef __linux__
  close(epfd);
  #endif
}

int CqDemultiplexer::init() {
  cq = stack->get_cqs()[work_num];
  #ifdef __linux__
  fabric = stack->get_fabric();
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
  #endif
  return 0;
}

int CqDemultiplexer::wait_event() {
  struct fid *fids[1];
  fids[0] = &cq->fid;
  #ifdef __linux__
  if (fi_trywait(fabric, fids, 1) == FI_SUCCESS) {
    int epoll_ret = epoll_wait(epfd, &event, 1, 200);
    if (event.data.ptr != (void*)&cq->fid) {
      std::cout << "Epoll wait error." << std::endl;
    }
    if (epoll_ret <= 0) {
      return 0;
    }
  }
  #endif
  uint64_t start, end = 0;
  start = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  end = start;
  do {
    fi_cq_msg_entry entry{};
    int ret = fi_cq_read(cq, &entry, 1);
    if (ret < 0 && ret != -FI_EAGAIN) {
      fi_cq_err_entry err_entry{};
      int err_res = fi_cq_readerr(cq, &err_entry, entry.flags);
      if (err_res < 0) {
        perror("fi_cq_read");
      } else {
        const char *err_str = fi_cq_strerror(cq, err_entry.prov_errno, err_entry.err_data, nullptr, 0);
        std::cerr << "fi_cq_read: " << err_str << std::endl;
      }
      break;
    } else if (ret > 0) {
      auto *ck = (Chunk*)entry.op_context;
      auto *con = (MsgConnection*)ck->con;
      if (entry.flags & FI_RECV) {
        if (con->status < CONNECTED) {
          std::unique_lock<std::mutex> l(con->con_mtx);
          con->con_cv.wait(l, [con] { return con->status >= CONNECTED; });
          l.unlock();
        }
        con->recv((char*)ck->buffer, entry.len);
        if (con->get_recv_callback()) {
          (*con->get_recv_callback())(&ck->buffer_id, &entry.len);
          con->activate_recv_chunk(ck);
        }
      } else if (entry.flags & FI_SEND) {
        assert(con->get_send_callback());
        (*con->get_send_callback())(&ck->buffer_id, nullptr);
      } else if (entry.flags & FI_READ) {
        if (con->get_read_callback()) {
          (*con->get_read_callback())(&ck->buffer_id, &entry.len);
        }
      } else if (entry.flags & FI_WRITE) {
      } else {
      }
      start = end;
    }
    end = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  } while (end-start <= 25);
  return 0;
}
