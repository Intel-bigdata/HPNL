#include "HPNL/CQExternalDemultiplexer.h"

CQExternalDemultiplexer::CQExternalDemultiplexer(FIStack *stack_, fid_cq *cq_) : stack(stack_), cq(cq_), start(0), end(0) {}

CQExternalDemultiplexer::~CQExternalDemultiplexer() {
  close(epfd);
}

int CQExternalDemultiplexer::init() {
  fabric = stack->get_fabric();
  if ((epfd = epoll_create1(0)) == -1) {
    perror("epoll_create1");
    return -1;
  }
  memset((void*)&event, 0, sizeof event);
  if (fi_control(&cq->fid, FI_GETWAIT, (void*)&fd)) {
    perror("fi_control");
    return -1;
  }
  event.events = EPOLLIN;
  event.data.ptr = &cq->fid;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event) < 0) {
    perror("epoll_ctl");
    return -1;
  }
  return 0;
}

int CQExternalDemultiplexer::wait_event(fid_eq** eq, Chunk** ck, int* rdma_buffer_id, int* block_buffer_size) {
  struct fid *fids[1];
  fids[0] = &cq->fid;
  int ret = 0;
  if (end - start >= 200) {
    if (fi_trywait(fabric, fids, 1) == FI_SUCCESS) {
      int epoll_ret = epoll_wait(epfd, &event, 1, 200);
      if (epoll_ret > 0) {
        assert(event.data.ptr == (void*)&cq->fid);
      } else if (epoll_ret < 0) {
        if (errno != EINTR) {
          perror("epoll_wait");
          return -1;
        }
        return 0;
      } else {
        return 0; 
      }
    }
    start = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
  }
  fi_cq_msg_entry entry;
  do {
    ret = fi_cq_read(cq, &entry, 1);
    if (ret == -FI_EAVAIL) {
      fi_cq_err_entry err_entry;
      fi_cq_readerr(cq, &err_entry, entry.flags);
      end = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
      perror("fi_cq_read");
      if (err_entry.err == FI_EOVERRUN) {
        return -1;
      }
      return 0;
    } else if (ret == -FI_EAGAIN) {
      end = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
      return 0;
    } else {
      end = start;
      *ck = (Chunk*)entry.op_context;
	  *rdma_buffer_id = (*ck)->rdma_buffer_id;
	  FIConnection *conn = (FIConnection*)(*ck)->con;
      if (!conn) {
        return 0;
      }
	  fid_eq *eq_tmp = (fid_eq*)conn->get_eqhandle()->get_ctx();
	  *eq = eq_tmp;
	  if (entry.flags & FI_RECV) {
		if(conn->status < CONNECTED){
			std::unique_lock<std::mutex> l(conn->con_mtx);
			conn->con_cv.wait(l, [conn] { return conn->status >= CONNECTED; });
			l.unlock();
		}
		conn->recv((char*)(*ck)->buffer, entry.len);
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
  } while (ret > 0);
  return 0;
}
