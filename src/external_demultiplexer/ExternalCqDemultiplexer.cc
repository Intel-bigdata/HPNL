#include "external_demultiplexer/ExternalCqDemultiplexer.h"
#include "core/MsgStack.h"
#include "core/MsgConnection.h"
#include "demultiplexer/EventType.h"

ExternalCqDemultiplexer::ExternalCqDemultiplexer(MsgStack *stack_, fid_cq *cq_) : stack(stack_), cq(cq_) {}

ExternalCqDemultiplexer::~ExternalCqDemultiplexer() {
}

int ExternalCqDemultiplexer::init() {
  return 0;
}

int ExternalCqDemultiplexer::wait_event(fid_eq** eq, Chunk** ck, int* buffer_id, int* block_buffer_size) {
	int ret = 0;
	fi_cq_msg_entry entry;
	ret = fi_cq_read(cq, &entry, 1);
	if (ret > 0) {
		*ck = (Chunk*)entry.op_context;
		*buffer_id = (*ck)->buffer_id;
		MsgConnection *conn = (MsgConnection*)(*ck)->con;
		if (!conn) {
			return 0;
		}
		fid_eq *eq_tmp = conn->get_eq();
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
		}
		if (entry.flags & FI_SEND) {
			return SEND_EVENT;
		}
		if (entry.flags & FI_READ) {
			return READ_EVENT;
		}
		if (entry.flags & FI_WRITE) {
			return WRITE_EVENT;
		}
		return 0;
	}

	if (ret == -FI_EAVAIL) {
		fi_cq_err_entry err_entry;
		fi_cq_readerr(cq, &err_entry, entry.flags);
		perror("fi_cq_read");
		if (err_entry.err == FI_EOVERRUN) {
			return -1;
		}
		return 0;
	}
	return 0;
}
