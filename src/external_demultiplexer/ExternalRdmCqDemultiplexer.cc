#include <assert.h>

#include <iostream>
#include <chrono>

#include "external_demultiplexer/ExternalRdmCqDemultiplexer.h"
#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "demultiplexer/EventType.h"

ExternalRdmCqDemultiplexer::ExternalRdmCqDemultiplexer(RdmStack *stack_) : stack(stack_) {
	cq = stack->get_cq();
}

ExternalRdmCqDemultiplexer::~ExternalRdmCqDemultiplexer() {
}

int ExternalRdmCqDemultiplexer::init() {
  return 0;
}

int ExternalRdmCqDemultiplexer::wait_event(Chunk** ck, int *buffer_id, int *block_buffer_size) {
  int ret = 0;
  fi_cq_msg_entry entry;
  ret = fi_cq_read(cq, &entry, 1);
  if(ret > 0){
    if (entry.flags & FI_RECV) {
      fi_context2 *ctx = (fi_context2*)entry.op_context;
      *ck = (Chunk*)ctx->internal[4];
      *block_buffer_size = entry.len;
      return RECV_EVENT;
    }
    if (entry.flags & FI_SEND) {
      fi_context2 *ctx = (fi_context2*)entry.op_context;
      if (ctx->internal[4] != NULL) {
    	  *ck = (Chunk*)ctx->internal[4];
    	  *block_buffer_size = entry.len;
      } else {
    	  *ck = (Chunk*)ctx->internal[5];
    	  assert((*ck) != NULL);
    	  if((*ck)->ctx_id < 0){
    		  delete (*ck);
    		  *ck = NULL;
    		  return 0;
    	  }
    	  *block_buffer_size = (*ck)->ctx_id;
      }
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
