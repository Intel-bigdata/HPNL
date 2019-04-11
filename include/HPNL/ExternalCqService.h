#ifndef EXTERNALCQSERVICE_H
#define EXTERNALCQSERVICE_H

#include <rdma/fi_domain.h>

#include "HPNL/Common.h"
#include "HPNL/ExternalEqService.h"
#include "HPNL/Connection.h"
#include "HPNL/FIConnection.h"
#include "HPNL/FIStack.h"
#include "HPNL/CQExternalDemultiplexer.h"

class ExternalCqService {
  public:
    ExternalCqService(ExternalEqService *service_, FIStack *stack_) : service(service_), stack(stack_) {}
    ~ExternalCqService() {
      for (int i = 0; i < service->get_worker_num(); i++) {
        delete cq_demulti_plexer[i];
      }
    }
    int init() {
      int i = 0;
      for (; i < service->get_worker_num(); i++) {
        cq_demulti_plexer[i] = new CQExternalDemultiplexer(stack, stack->get_cqs()[i]);
        if (cq_demulti_plexer[i]->init() == -1) {
          break;
        }
      }
      if (i < service->get_worker_num()) {
        for (int j = 0; j <= i; j++) {
          delete cq_demulti_plexer[j];
        }
        return -1;
      }
      return 0; 
    }
    int wait_cq_event(int num, fid_eq** eq, Chunk** ck, int* rdma_buffer_id, int* block_buffer_size) {
      return cq_demulti_plexer[num]->wait_event(eq, ck, rdma_buffer_id, block_buffer_size);
    }
    Connection* get_connection(fid_eq* eq) {
      return stack->get_connection(&eq->fid);
    }
    Chunk* get_chunk(int id, int type) {
      return service->get_chunk(id, type); 
    }
  private:
    ExternalEqService *service;
    FIStack *stack;
    CQExternalDemultiplexer *cq_demulti_plexer[MAX_WORKERS];
};

#endif
