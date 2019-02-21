#include "HPNL/ExternalCqService.h"
#include "HPNL/ExternalEqService.h"

#include "com_intel_hpnl_core_CqService.h"

static jfieldID _get_self_id(JNIEnv *env, jobject thisObj)
{
  static int init = 0;
  static jfieldID fidSelfPtr;
  if(!init)
  {
    jclass thisClass = env->GetObjectClass(thisObj);
    fidSelfPtr = env->GetFieldID(thisClass, "nativeHandle", "J");
    init = 1;
  }
  return fidSelfPtr;
}

static ExternalCqService*_get_self(JNIEnv *env, jobject thisObj)
{
  jlong selfPtr = env->GetLongField(thisObj, _get_self_id(env, thisObj));
  return *(ExternalCqService**)&selfPtr;
}

static void _set_self(JNIEnv *env, jobject thisObj, ExternalCqService *self)
{
  jlong selfPtr = *(jlong*)&self;
  env->SetLongField(thisObj, _get_self_id(env, thisObj), selfPtr);
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    wait_cq_event
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_CqService_wait_1cq_1event(JNIEnv *env, jobject thisObj, jint index) {
  ExternalCqService *externalCqService = _get_self(env, thisObj);
  fid_eq *eq;
  int block_buffer_size = 0;
  int rdma_buffer_id = 0;
  int ret = externalCqService->wait_cq_event(index, &eq, &rdma_buffer_id, &block_buffer_size);
  if (ret <= 0) {
    return ret; 
  }
  Connection *con = externalCqService->get_connection(eq);
  if (!con)
    return -1;
  assert(thisObj);
  jclass thisClass = (*env).GetObjectClass(thisObj);
  jmethodID handleCqCallback = (*env).GetMethodID(thisClass, "handleCqCallback", "(JIII)V");
  jlong jEq = *(jlong*)&eq;
  (*env).CallVoidMethod(thisObj, handleCqCallback, jEq, ret, rdma_buffer_id, block_buffer_size);
  if (ret == RECV_EVENT) {
    Chunk *ck = externalCqService->get_chunk(rdma_buffer_id, RECV_CHUNK);
    if (con->activate_chunk(ck)) {
      // TODO: error handler 
    }
  } else if (ret == SEND_EVENT) {
    Chunk *ck = externalCqService->get_chunk(rdma_buffer_id, SEND_CHUNK); 
  }
  return ret;
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_CqService_init(JNIEnv *env, jobject thisObj, jlong service) {
  ExternalEqService *externalEqService = *(ExternalEqService**)&service;
  ExternalCqService *externalCqService = new ExternalCqService(externalEqService, externalEqService->get_stack());
  _set_self(env, thisObj, externalCqService);
  return externalCqService->init();
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    finalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_CqService_finalize(JNIEnv *env, jobject thisObj) {
  ExternalCqService *externalCqService = _get_self(env, thisObj);
  if (externalCqService != NULL) {
    delete externalCqService;
    _set_self(env, thisObj, NULL);
  }
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    free
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_CqService_free(JNIEnv *env, jobject thisObj) {
  ExternalCqService *externalCqService = _get_self(env, thisObj);
  if (externalCqService != NULL) {
    delete externalCqService;
    _set_self(env, thisObj, NULL);
  }
}

