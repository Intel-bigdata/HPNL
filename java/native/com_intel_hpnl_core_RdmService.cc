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

#include "com_intel_hpnl_core_RdmService.h"
#include "demultiplexer/EventType.h"
#include "external_service/ExternalRdmService.cc"

#include <iostream>

static jlong selfPtr;
static jmethodID handleCallback;
static jmethodID reallocBufferPool;
static jmethodID establishConnection;
static jmethodID pushSendBuffer;

static jfieldID _get_self_id(JNIEnv* env, jobject thisObj) {
  static int init = 0;
  static jfieldID fidSelfPtr;
  if (!init) {
    jclass serviceClassTmp;
    serviceClassTmp = env->FindClass("com/intel/hpnl/core/RdmService");

    handleCallback = (*env).GetMethodID(serviceClassTmp, "handleCallback", "(JIII)V");
    reallocBufferPool = (*env).GetMethodID(serviceClassTmp, "reallocBufferPool", "()V");
    establishConnection =
        (*env).GetMethodID(serviceClassTmp, "establishConnection", "(J)V");
    pushSendBuffer = (*env).GetMethodID(serviceClassTmp, "pushSendBuffer", "(JI)V");

    fidSelfPtr = env->GetFieldID(serviceClassTmp, "nativeHandle", "J");
    init = 1;
  }
  return fidSelfPtr;
}

static ExternalRdmService* _get_self(JNIEnv* env, jobject thisObj) {
  jlong selfPtr = env->GetLongField(thisObj, _get_self_id(env, thisObj));
  return *(ExternalRdmService**)&selfPtr;
}

static void _set_self(JNIEnv* env, jobject thisObj, ExternalRdmService* self) {
  jlong selfPtr = *(jlong*)&self;
  env->SetLongField(thisObj, _get_self_id(env, thisObj), selfPtr);
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmService_init(JNIEnv* env, jobject obj,
                                                                jint worker_num,
                                                                jint buffer_num,
                                                                jboolean is_server) {
  ExternalRdmService* service = new ExternalRdmService(worker_num, buffer_num, is_server);
  int res = service->init();
  _set_self(env, obj, service);
  return res;
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmService_listen(
    JNIEnv* env, jobject obj, jstring ip_, jstring port_, jlong nativeHandle) {
  ExternalRdmService* service = *(ExternalRdmService**)&nativeHandle;
  const char* ip = (*env).GetStringUTFChars(ip_, 0);
  const char* port = (*env).GetStringUTFChars(port_, 0);
  RdmConnection* con = service->listen(ip, port);
  if (!con) {
    (*env).CallVoidMethod(obj, reallocBufferPool);
    con = service->listen(ip, port);
    if (!con) {
      return -1;
    }
  }

  (*env).CallVoidMethod(obj, establishConnection, *(jlong*)&con);

  std::vector<Chunk*> send_buffer = con->get_send_chunk();
  if (!send_buffer.empty()) {
    int chunks_size = send_buffer.size();
    for (int i = 0; i < chunks_size; i++) {
      (*env).CallVoidMethod(obj, pushSendBuffer, *(jlong*)&con,
                            send_buffer[i]->buffer_id);
    }
  }
  return 0;
}

JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_RdmService_get_1con(
    JNIEnv* env, jobject obj, jstring ip_, jstring port_, jlong nativeHandle) {
  ExternalRdmService* service = *(ExternalRdmService**)&nativeHandle;
  const char* ip = (*env).GetStringUTFChars(ip_, 0);
  const char* port = (*env).GetStringUTFChars(port_, 0);
  RdmConnection* con = (RdmConnection*)service->get_con(ip, port);
  if (!con) {
    (*env).CallVoidMethod(obj, reallocBufferPool);
    con = (RdmConnection*)service->get_con(ip, port);
    if (!con) {
      return -1;
    }
  }
  (*env).CallVoidMethod(obj, establishConnection, *(jlong*)&con);

  std::vector<Chunk*> send_buffer = con->get_send_chunk();
  int chunks_size = send_buffer.size();
  for (int i = 0; i < chunks_size; i++) {
    (*env).CallVoidMethod(obj, pushSendBuffer, *(jlong*)&con, send_buffer[i]->buffer_id);
  }
  return *(jlong*)&con;
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmService_wait_1event(
    JNIEnv* env, jobject obj, jint index, jlong nativeHandle) {
  ExternalRdmService* service = *(ExternalRdmService**)&nativeHandle;
  Chunk* ck = nullptr;
  int block_buffer_size = 0;
  int ret = service->wait_event(&ck, &block_buffer_size, index);
  if (ret == 0) {
    return 0;
  } else if (ret == CLOSE_EVENT) {
    return -1;
  } else {
    RdmConnection* con = (RdmConnection*)ck->con;
    (*env).CallVoidMethod(obj, handleCallback, *(jlong*)&con, ret, ck->buffer_id,
                          block_buffer_size);
    if (ret == RECV_EVENT) {
      con->activate_recv_chunk(ck, index);
    }
    return ret;
  }
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmService_set_1buffer1(
    JNIEnv* env, jobject obj, jobject buffer, jlong size, jint bufferId,
    jlong nativeHandle) {
  ExternalRdmService* service = *(ExternalRdmService**)&nativeHandle;
  jbyte* bufferAddr = (jbyte*)(*env).GetDirectBufferAddress(buffer);
  assert(bufferAddr != NULL);
  service->set_buffer((char*)bufferAddr, size, bufferId);
}

/*
 * Class:     com_intel_hpnl_core_RdmService
 * Method:    free
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmService_free(JNIEnv* env, jobject obj,
                                                                jlong nativeHandle) {
  ExternalRdmService* service = *(ExternalRdmService**)&nativeHandle;
  if (service != NULL) {
    delete service;
    service = NULL;
    _set_self(env, obj, NULL);
  }
}
