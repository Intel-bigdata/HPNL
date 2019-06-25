#include "demultiplexer/Proactor.h"
#include "demultiplexer/EventType.h"
#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/CqDemultiplexer.h"
#include "demultiplexer/RdmCqDemultiplexer.h"
#include "demultiplexer/EventHandler.h"

Proactor::Proactor(EqDemultiplexer *eqDemultiplexer_, CqDemultiplexer **cqDemultiplexer_, int cq_worker_num_) : eqDemultiplexer(eqDemultiplexer_), cq_worker_num(cq_worker_num_) {
  for (int i = 0; i < cq_worker_num; i++) {
    cqDemultiplexer[i] = *(cqDemultiplexer_+i);
  }
}

Proactor::Proactor(RdmCqDemultiplexer *rdmCqDemultiplexer_) : rdmCqDemultiplexer(rdmCqDemultiplexer_) {}

Proactor::~Proactor() {
  eventMap.erase(eventMap.begin(), eventMap.end()); 
}

int Proactor::eq_service() {
  if (eqDemultiplexer != NULL) {
    return eqDemultiplexer->wait_event(eventMap);
  }
  return 0;
}

int Proactor::cq_service(int index) {
  if (cqDemultiplexer[index] != NULL) {
    return cqDemultiplexer[index]->wait_event();
  }
  return 0;
}

int Proactor::rdm_cq_service() {
  return rdmCqDemultiplexer->wait_event();
}

int Proactor::register_handler(std::shared_ptr<EventHandler> eh) {
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
  std::map<fid*, std::shared_ptr<EventHandler>>::iterator iter = eventMap.find(id);
  if (iter != eventMap.end()) {
    eventMap.erase(iter);
    return eqDemultiplexer->remove_event(id);
  }
  else {
    return -1;
  }
}

