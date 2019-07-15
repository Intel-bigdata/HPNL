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

#ifndef THREADWRAPPER_H
#define THREADWRAPPER_H

#include <assert.h>

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

class ThreadWrapper {
 public:
  ThreadWrapper() : done(false) {}
  virtual ~ThreadWrapper() = default;
  void join() {
    if (thread.joinable()) {
      thread.join();
    } else {
      std::unique_lock<std::mutex> l(join_mutex);
      join_event.wait(l, [=] { return done.load(); });
    }
  }
  void start(bool background_thread = false) {
    thread = std::thread(&ThreadWrapper::thread_body, this);
    if (background_thread) {
      thread.detach();
    }
  }
  void stop() { done.store(true); }
  void set_affinity(int cpu) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    int res = pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
    if (res) {
      abort();
    }
#endif
  }
  void thread_body() {
    try {
      while (true) {
        int ret = entry();
        if (done.load() || ret == -1) {
          if (!thread.joinable()) {
            join_event.notify_all();
          }
          break;
        }
      }
    } catch (ThreadAbortException&) {
      abort();
    } catch (std::exception& ex) {
      ExceptionCaught(ex);
    } catch (...) {
      UnknownExceptionCaught();
    }
  }

 private:
  class ThreadAbortException : std::exception {};

 protected:
  virtual int entry() = 0;
  virtual void abort() = 0;
  virtual void ExceptionCaught(std::exception& exception) {}
  virtual void UnknownExceptionCaught() {}

 private:
  std::thread thread;
  std::mutex join_mutex;
  std::condition_variable join_event;
  std::atomic_bool done = {false};
};

#endif
