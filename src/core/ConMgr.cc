#include "core/ConMgr.h"

ConMgr::~ConMgr() {
  while (!event_queue.empty()) {
    ConEvent *event = event_queue.back();
    event_queue.pop_back();
    delete event;
  }
}

void ConMgr::handle_event() {
  std::unique_lock<std::mutex> lq(queue_mtx); 
  while (event_queue.empty()) {
    queue_cv.wait(lq); 
  }
  ConEvent *event = event_queue.back();
  event_queue.pop_back();
  lq.unlock();

  std::unique_lock<std::mutex> le(event_mtx);
  event_cv.wait(le, [&]{ return ready; });

  if (event->type == CONNECT) {
    connect(event->ep, event->addr); 
  } else if (event->type == ACCEPT) {
    accept(event->ep); 
  } else if (event->type == SHUTDOWN) {
    shutdown(event->ep); 
  } else {
  
  }
  ready = false;
  delete event;
  le.unlock();
}

void ConMgr::notify() {
  std::unique_lock<std::mutex> le(event_mtx);
  ready = true;
  le.unlock();
  event_cv.notify_one();
}

void ConMgr::push_event(fid_ep* ep, EventType type, void* addr) {
  ConEvent *event = new ConEvent(ep, type, addr);
  std::unique_lock<std::mutex> lq(queue_mtx); 
  bool is_empty = event_queue.empty();
  event_queue.push_front(event);
  lq.unlock();
  if (is_empty) {
    queue_cv.notify_one();
  }
}

void ConMgr::connect(fid_ep* ep, void* addr) {
  fi_connect(ep, addr, NULL, 0);
}

void ConMgr::accept(fid_ep* ep) {
  fi_accept(ep, NULL, 0);
}

void ConMgr::shutdown(fid_ep* ep) {
  fi_shutdown(ep, 0);
}

