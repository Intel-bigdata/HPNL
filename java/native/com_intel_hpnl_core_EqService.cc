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

#include <rdma/fi_domain.h>

#include "com_intel_hpnl_core_EqService.h"

#include <assert.h>

#include "core/MsgConnection.h"
#include "demultiplexer/EventType.h"
#include "external_service/ExternalEqService.h"

#include <iostream>

static jlong selfPtr;
static jmethodID handleEqCallback;
static jmethodID reallocBufferPool;
static jmethodID establishConnection;
static jmethodID closeConnection;
static jmethodID pushSendBuffer;

static jfieldID _get_self_id(JNIEnv* env, jobject thisObj) {
  static int init = 0;
  static jfieldID fidSelfPtr;
  if (!init) {
    jclass eqServiceClassTmp;
    eqServiceClassTmp = env->FindClass("com/intel/hpnl/core/EqService");

    handleEqCallback =
        (*env).GetMethodID(eqServiceClassTmp, "handleEqCallback", "(JII)V");
    reallocBufferPool = (*env).GetMethodID(eqServiceClassTmp, "reallocBufferPool", "()V");
    establishConnection =
        (*env).GetMethodID(eqServiceClassTmp, "establishConnection",
                           "(JJILjava/lang/String;ILjava/lang/String;I)V");
    closeConnection = (*env).GetMethodID(eqServiceClassTmp, "closeConnection", "(J)V");
    pushSendBuffer = (*env).GetMethodID(eqServiceClassTmp, "pushSendBuffer", "(JI)V");

    fidSelfPtr = env->GetFieldID(eqServiceClassTmp, "nativeHandle", "J");
    init = 1;
  }
  return fidSelfPtr;
}

static ExternalEqService* _get_self(JNIEnv* env, jobject thisObj) {
  jlong selfPtr = env->GetLongField(thisObj, _get_self_id(env, thisObj));
  return *(ExternalEqService**)&selfPtr;
}

static void _set_self(JNIEnv* env, jobject thisObj, ExternalEqService* self) {
  jlong selfPtr = *(jlong*)&self;
  env->SetLongField(thisObj, _get_self_id(env, thisObj), selfPtr);
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_EqService_init(JNIEnv* env,
                                                               jobject thisObj,
                                                               jint worker_num_,
                                                               jint buffer_num_,
                                                               jboolean is_server_) {
  const bool is_server = (bool)is_server_;
  ExternalEqService* service = new ExternalEqService(worker_num_, buffer_num_, is_server);
  _set_self(env, thisObj, service);
  return service->init();
}

/*
 * Class:     com_intel_hpnl_EqService
 * Method:    finalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_finalize(JNIEnv* env,
                                                                   jobject thisObj) {}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_free(JNIEnv* env,
                                                               jobject thisObj,
                                                               jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  if (service != NULL) {
    delete service;
    service = NULL;
    _set_self(env, thisObj, NULL);
  }
}

/*
 * Class:     com_intel_hpnl_EqService
 * Method:    connect
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_EqService_native_1connect(
    JNIEnv* env, jobject thisObj, jstring ip_, jstring port_, jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  const char* ip = (*env).GetStringUTFChars(ip_, 0);
  const char* port = (*env).GetStringUTFChars(port_, 0);
  
  while (!service->is_buffer_enough()) {
    (*env).CallVoidMethod(thisObj, reallocBufferPool);
  }
  fid_eq* new_eq = service->connect(ip, port);
  if (!new_eq) {
    return -1;
  }
  jlong ret = *(jlong*)&new_eq;
  return ret;
}

/*
 * Class:     com_intel_hpnl_EqService
 * Method:    wait_eq_event
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_EqService_wait_1eq_1event1(
    JNIEnv* env, jobject thisObj, jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  fi_info* info = NULL;
  fid_eq* eq;
  MsgConnection* con = NULL;
  int ret = service->wait_eq_event(&info, &eq, &con);
  if (ret < 0) return ret;
  if (ret == ACCEPT_EVENT) {
    // accept new connection and register eq id
    while (!service->is_buffer_enough()) {
      (*env).CallVoidMethod(thisObj, reallocBufferPool);
    }
    fid_eq* new_eq = service->accept(info);
    assert(new_eq != NULL);
    service->add_eq_event(new_eq);
  } else if (ret == CONNECTED_EVENT) {
    char** dest_addr = (char**)malloc(sizeof(char*));
    size_t dest_port;
    char** src_addr = (char**)malloc(sizeof(char*));
    size_t src_port;
    assert(con);
    con->get_addr(dest_addr, &dest_port, src_addr, &src_port);
    jstring dest_addr_str = (*env).NewStringUTF(*dest_addr);
    jstring src_addr_str = (*env).NewStringUTF(*src_addr);
    free(dest_addr);
    free(src_addr);
    // register connection
    jlong jEq = *(jlong*)&eq;
    jlong jCon = *(jlong*)&con;
    (*env).CallVoidMethod(thisObj, establishConnection, jEq, jCon, con->get_cq_index(),
                          dest_addr_str, dest_port, src_addr_str, src_port);

    // set send buffer;
    std::vector<Chunk*> send_buffer = con->get_send_chunks();
    int chunks_size = send_buffer.size();
    for (int i = 0; i < chunks_size; i++) {
      (*env).CallVoidMethod(thisObj, pushSendBuffer, jEq, send_buffer[i]->buffer_id);
    }

    // callback
    (*env).CallVoidMethod(thisObj, handleEqCallback, jEq, ret, 0);
    {
      std::lock_guard<std::mutex> l(con->con_mtx);
      con->status = CONNECTED;
    }
    con->con_cv.notify_one();
  } else if (ret == SHUTDOWN) {
    jlong jEq = *(jlong*)&eq;
    (*env).CallVoidMethod(thisObj, closeConnection, jEq);
  } else {
  }
  return ret;
}

/*
 * Class:     com_intel_hpnl_core_EqService
 * Method:    add_eq_event
 * Signature: (j)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_EqService_add_1eq_1event(
    JNIEnv* env, jobject thisObj, jlong eqPtr, jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  fid_eq* eq = *(fid_eq**)&eqPtr;
  return service->add_eq_event(eq);
}

/*
 * Class:     com_intel_hpnl_core_EqService
 * Method:    delete_eq_event
 * Signature: (j)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_EqService_delete_1eq_1event1(
    JNIEnv* env, jobject thisObj, jlong eqPtr, jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  if (service == NULL) return 0;
  fid_eq* eq = *(fid_eq**)&eqPtr;
  return service->delete_eq_event(eq);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_shutdown1(JNIEnv* env,
                                                                    jobject thisObj,
                                                                    jlong eqPtr,
                                                                    jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  fid_eq* eq = *(fid_eq**)&eqPtr;
  MsgConnection* con = (MsgConnection*)service->get_connection(eq);
  if (!con) {
    return;
  }
  if (con->status < DOWN) {
    con->shutdown();
    con->status = DOWN;
    service->reap(&eq->fid);
  }
}

/*
 * Class:     com_intel_hpnl_EqService
 * Method:    set_send_buffer
 * Signature: (Ljava/nio/ByteBuffer;J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_set_1buffer1(
    JNIEnv* env, jobject thisObj, jobject buffer, jlong size, jint bufferId,
    jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  jbyte* bufferAddr = (jbyte*)(*env).GetDirectBufferAddress(buffer);
  service->set_buffer((char*)bufferAddr, size, bufferId);
}

/*
 * Class:     com_intel_hpnl_EqService
 * Method:    reg_rma_buffer
 * Signature: (Ljava/nio/ByteBuffer;J)V
 */
JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_EqService_reg_1rma_1buffer(
    JNIEnv* env, jobject thisObj, jobject send_buffer, jlong size, jint bufferId,
    jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(send_buffer);
  return service->reg_rma_buffer((char*)buffer, size, bufferId);
}

JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_EqService_reg_1rma_1buffer_1by_1address(
    JNIEnv* env, jobject thisObj, jlong address, jlong size, jint bufferId,
    jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  return service->reg_rma_buffer(*(char**)&address, size, bufferId);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_unreg_1rma_1buffer(
    JNIEnv* env, jobject thisObj, jint bufferId, jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  service->unreg_rma_buffer(bufferId);
}

/*
 * Class:     com_intel_hpnl_core_EqService
 * Method:    get_buffer_address
 * Signature: (Ljava/nio/ByteBuffer)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_EqService_get_1buffer_1address(
    JNIEnv* env, jobject thisObj, jobject rma_buffer, jlong eqServicePtr) {
  ExternalEqService* service = *(ExternalEqService**)&eqServicePtr;
  assert(rma_buffer);
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(rma_buffer);
  jlong buffer_address = *(jlong*)&buffer;
  return buffer_address;
}
