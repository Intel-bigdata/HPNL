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

#include "demultiplexer/Proactor.h"
#include "demultiplexer/EventType.h"
#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/CqDemultiplexer.h"
#include "demultiplexer/RdmCqDemultiplexer.h"
#include "demultiplexer/EventHandler.h"

Proactor::Proactor(EqDemultiplexer *eqDemultiplexer_, CqDemultiplexer **cqDemultiplexer_, int cq_worker_num_) :
  eqDemultiplexer(eqDemultiplexer_), cq_worker_num(cq_worker_num_), rdmCqDemultiplexer(nullptr) {
  for (int i = 0; i < cq_worker_num; i++) {
    cqDemultiplexer[i] = *(cqDemultiplexer_+i);
  }
}

Proactor::Proactor(RdmCqDemultiplexer *rdmCqDemultiplexer_) : rdmCqDemultiplexer(rdmCqDemultiplexer_) {
  eqDemultiplexer = nullptr;
  cq_worker_num = 0;
}

Proactor::~Proactor() {
  eventMap.erase(eventMap.begin(), eventMap.end()); 
}

int Proactor::eq_service() {
  int res = 0;
  if (eqDemultiplexer != nullptr) {
    {
      std::lock_guard<std::mutex> l(mtx);
      for (const auto& event : eventMap) {
        curEventMap[event.first] = event.second;
      }
    }
    res = eqDemultiplexer->wait_event(curEventMap);
    curEventMap.clear();
  }
  return res;
}

int Proactor::cq_service(int index) {
  if (cqDemultiplexer[index] != nullptr) {
    return cqDemultiplexer[index]->wait_event();
  }
  return 0;
}

int Proactor::rdm_cq_service() {
  return rdmCqDemultiplexer->wait_event();
}

int Proactor::register_handler(std::shared_ptr<EventHandler> eh) {
  std::lock_guard<std::mutex> l(mtx);
  fid_eq *eq = eh->get_handle();
  if (eventMap.find(&eq->fid) == eventMap.end()) {
    eventMap.insert(std::make_pair(&eq->fid, eh)); 
  }
  return eqDemultiplexer->register_event(&eq->fid);
}

int Proactor::remove_handler(std::shared_ptr<EventHandler> eh) {
  fid_eq *eq = eh->get_handle();
  return remove_handler(&eq->fid);
}

int Proactor::remove_handler(fid* id) {
  std::lock_guard<std::mutex> l(mtx);
  auto iter = eventMap.find(id);
  if (iter != eventMap.end()) {
    eventMap.erase(iter);
    return eqDemultiplexer->remove_event(id);
  }
  else {
    return -1;
  }
}

