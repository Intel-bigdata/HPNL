// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#ifndef EXTERNALCQSERVICE_H
#define EXTERNALCQSERVICE_H

#include <rdma/fi_domain.h>

#include "HPNL/Common.h"
#include "HPNL/Connection.h"
#include "core/MsgConnection.h"
#include "core/MsgStack.h"
#include "external_demultiplexer/ExternalCqDemultiplexer.h"
#include "external_service/ExternalEqService.h"

class ExternalCqService {
 public:
  ExternalCqService(ExternalEqService* service_, MsgStack* stack_)
      : service(service_), stack(stack_) {}
  ~ExternalCqService() {
    for (int i = 0; i < service->get_worker_num(); i++) {
      delete cq_demultiplexer[i];
    }
  }
  int init() {
    int i = 0;
    for (; i < service->get_worker_num(); i++) {
      cq_demultiplexer[i] = new ExternalCqDemultiplexer(stack, stack->get_cqs()[i]);
      if (cq_demultiplexer[i]->init() == -1) {
        break;
      }
    }
    if (i < service->get_worker_num()) {
      for (int j = 0; j <= i; j++) {
        delete cq_demultiplexer[j];
      }
      return -1;
    }
    return 0;
  }
  int wait_cq_event(int num, fid_eq** eq, Chunk** ck, int* buffer_id,
                    int* block_buffer_size) {
    return cq_demultiplexer[num]->wait_event(eq, ck, buffer_id, block_buffer_size);
  }
  Connection* get_connection(fid_eq* eq) { return stack->get_connection(&eq->fid); }

 private:
  ExternalEqService* service;
  MsgStack* stack;
  ExternalCqDemultiplexer* cq_demultiplexer[MAX_WORKERS]{};
};

#endif
