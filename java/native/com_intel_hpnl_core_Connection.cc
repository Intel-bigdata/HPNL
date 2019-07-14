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

#include "com_intel_hpnl_core_Connection.h"

#include  "core/ConnectionImpl.h"
#include "core/MsgStack.h"

static jfieldID _get_self_id(JNIEnv *env, jobject thisObj)
{
  static int init = 0;
  static jfieldID fidSelfPtr;
  if (!init) {
    jclass thisClass = env->GetObjectClass(thisObj);
    fidSelfPtr = env->GetFieldID(thisClass, "nativeHandle", "J");
    init = 1;
  }
  return fidSelfPtr;
}

static void _set_self(JNIEnv *env, jobject thisObj, long nativeCon)
{
  env->SetLongField(thisObj, _get_self_id(env, thisObj), nativeCon);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_recv(JNIEnv *env, jobject thisObj, jobject bufferObj, jint mid, jlong conPtr) {
  ConnectionImpl *con = *(ConnectionImpl**)&conPtr;
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(bufferObj);
  con->recv((char*)buffer, mid);
}

JNIEXPORT int JNICALL Java_com_intel_hpnl_core_Connection_send(JNIEnv *env, jobject thisObj, jint blockBufferSize, jint bufferId, jlong conPtr) {
  ConnectionImpl *con = *(ConnectionImpl**)&conPtr;
  return con->send(blockBufferSize, bufferId);
}

JNIEXPORT int JNICALL Java_com_intel_hpnl_core_Connection_read(JNIEnv *env, jobject thisObj, jint bufferId, jint localOffset, jlong len, jlong remoteAddr, jlong remoteMr, jlong conPtr) {
  ConnectionImpl *con = *(ConnectionImpl**)&conPtr;
  return con->read(bufferId, localOffset, len, remoteAddr, remoteMr);
}

/*
 * Class:     com_intel_hpnl_Connection
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_init(JNIEnv *env, jobject thisObj, jlong nativeCon) {
  _set_self(env, thisObj, nativeCon);
}

/*
 * Class:     com_intel_hpnl_Connection
 * Method:    finalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_finalize(JNIEnv *env, jobject thisObj) {
}
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_free(JNIEnv *env, jobject thisObj, jlong conPtr) {
  ConnectionImpl *con = *(ConnectionImpl**)&conPtr;
  if (con != NULL) {
    delete con;
    con = NULL;
  }
}
