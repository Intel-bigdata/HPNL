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

#include <iostream>
#include "com_intel_hpnl_core_RdmConnection.h"
#include "core/RdmConnection.h"

static jfieldID _get_self_id(JNIEnv* env, jobject thisObj) {
  static int init = 0;
  static jfieldID fidSelfPtr;
  if (!init) {
    jclass thisClass = env->GetObjectClass(thisObj);
    fidSelfPtr = env->GetFieldID(thisClass, "nativeHandle", "J");
    init = 1;
  }
  return fidSelfPtr;
}

static void _set_self(JNIEnv* env, jobject thisObj, long nativeCon) {
  env->SetLongField(thisObj, _get_self_id(env, thisObj), nativeCon);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmConnection_init(JNIEnv* env,
                                                                   jobject obj,
                                                                   jlong nativeHandle) {
  _set_self(env, obj, nativeHandle);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmConnection_get_1local_1name(
    JNIEnv* env, jobject obj, jobject buffer, jlong nativeHandle) {
  RdmConnection* con = *(RdmConnection**)&nativeHandle;
  jbyte* bytes = (jbyte*)(*env).GetDirectBufferAddress(buffer);
  char* local_name = con->get_local_name();
  memcpy(bytes, local_name, con->get_local_name_length());
}

/*
 * Class:     com_intel_hpnl_core_RdmConnection
 * Method:    get_local_name_length
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_get_1local_1name_1length(
    JNIEnv* env, jobject obj, jlong nativeHandle) {
  RdmConnection* con = *(RdmConnection**)&nativeHandle;
  return con->get_local_name_length();
}

/*
 * Class:     com_intel_hpnl_core_RdmConnection
 * Method:    send
 * Signature: (IIJ)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_send(
    JNIEnv* env, jobject obj, jint blockBufferSize, jint bufferId, jlong nativeHandle) {
  RdmConnection* con = *(RdmConnection**)&nativeHandle;
  int res = con->send(blockBufferSize, bufferId);
  return res;
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_sendTo(
    JNIEnv* env, jobject obj, jint blockBufferSize, jint bufferId, jobject peerName,
    jlong nativeHandle) {
  RdmConnection* con = *(RdmConnection**)&nativeHandle;
  jbyte* bytes = (jbyte*)(*env).GetDirectBufferAddress(peerName);
  int res = con->sendTo(blockBufferSize, bufferId, (char*)bytes);
  return res;
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_sendBuf(
    JNIEnv* env, jobject obj, jobject buffer, jint bufferSize, jlong nativeHandle) {
  RdmConnection* con = *(RdmConnection**)&nativeHandle;
  jbyte* bytes = (jbyte*)(*env).GetDirectBufferAddress(buffer);
  int res = con->sendBuf((char*)bytes, bufferSize);
  return res;
}

/*
 * Class:     com_intel_hpnl_core_RdmConnection
 * Method:    sendBufTo
 * Signature: (Ljava/nio/ByteBuffer;I[BJ)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_sendBufTo(
    JNIEnv* env, jobject, jobject buffer, jint bufferSize, jobject peerName,
    jlong nativeHandle) {
  RdmConnection* con = *(RdmConnection**)&nativeHandle;
  jbyte* bytes = (jbyte*)(*env).GetDirectBufferAddress(buffer);
  jbyte* name = (jbyte*)(*env).GetDirectBufferAddress(peerName);
  int res = con->sendBufTo((char*)bytes, bufferSize, (char*)name);
  return res;
}
