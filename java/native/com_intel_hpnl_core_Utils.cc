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

#include "com_intel_hpnl_core_Utils.h"

#include <sched.h>

/*
 * Class:     com_intel_hpnl_core_Utils
 * Method:    set_affinity
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Utils_set_1affinity(JNIEnv* env,
                                                                    jclass cls,
                                                                    jlong affinity) {
#ifdef __linux__
  int i;
  cpu_set_t mask;
  CPU_ZERO(&mask);
  for (i = 0; i < sizeof(affinity) * 8; i++) {
    if ((affinity >> i) & 1) {
      CPU_SET(i, &mask);
    }
  }
  sched_setaffinity(0, sizeof(mask), &mask);
#endif
}
