#include <assert.h>

#include <iostream>
#include <chrono>

#include "external_demultiplexer/ExternalRdmCqDemultiplexer.h"
#include "core/RdmStack.h"
#include "core/RdmConnection.h"
#include "demultiplexer/EventType.h"

ExternalRdmCqDemultiplexer::ExternalRdmCqDemultiplexer(RdmStack *stack_) : stack(stack_) {
	cq = stack->get_cqs();
}

ExternalRdmCqDemultiplexer::~ExternalRdmCqDemultiplexer() {
}

int ExternalRdmCqDemultiplexer::init() {
  return 0;
}

int ExternalRdmCqDemultiplexer::wait_event(Chunk** ck, int *buffer_id, int *block_buffer_size) {
  return 0;
}
