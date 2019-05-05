#include "demultiplexer/Proactor.h"
#include "demultiplexer/EventType.h"
#include "demultiplexer/EqDemultiplexer.h"
#include "demultiplexer/CqDemultiplexer.h"
#include "demultiplexer/EventHandler.h"

Proactor::Proactor(EqDemultiplexer *eqDemultiplexer_, CqDemultiplexer **cqDemultiplexer_, int worker_num) : eqDemultiplexer(eqDemultiplexer_) {
  for (int i = 0; i < worker_num; i++) {
    cqDemultiplexer[i] = *(cqDemultiplexer_+i);
  }
}

Proactor::~Proactor() {
  eventMap.erase(eventMap.begin(), eventMap.end()); 
}

int Proactor::eq_service() {
  return eqDemultiplexer->wait_event(eventMap);
}

int Proactor::cq_service(int index) {
  return cqDemultiplexer[index]->wait_event();
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

