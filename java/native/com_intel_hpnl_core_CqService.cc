#include "demultiplexer/EventType.h"
#include "external_service/ExternalCqService.h"
#include "external_service/ExternalEqService.h"

#include "com_intel_hpnl_core_CqService.h"

static jmethodID handleCqCallback;
static jlong selfPtr;

static jfieldID _get_self_id(JNIEnv *env, jobject thisObj)
{
  static int init = 0;
  static jfieldID fidSelfPtr;
  if(!init)
  {
    jclass cqServiceClassTmp;
    cqServiceClassTmp = env->FindClass("com/intel/hpnl/core/CqService");
    handleCqCallback = (*env).GetMethodID(cqServiceClassTmp, "handleCqCallback", "(JIII)V");
    fidSelfPtr = env->GetFieldID(cqServiceClassTmp, "nativeHandle", "J");
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
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_CqService_wait_1cq_1event(JNIEnv *env, jobject thisObj, jint index, jlong cqServicePtr) {
  ExternalCqService *service = *(ExternalCqService**)&cqServicePtr;
  fid_eq *eq;
  int block_buffer_size = 0;
  int buffer_id = 0;
  MsgConnection *con = NULL;
  Chunk *ck = NULL;
  int ret = service->wait_cq_event(index, &eq, &ck, &buffer_id, &block_buffer_size);
  if (ret <= 0) {
    return ret; 
  }
  con = (MsgConnection*)ck->con;
  if (!con)
    return -1;
  jlong jEq = *(jlong*)&eq;
  (*env).CallVoidMethod(thisObj, handleCqCallback, jEq, ret, buffer_id, block_buffer_size);
  if (ret == RECV_EVENT) {
    if (con->activate_recv_chunk(ck)) {
      // TODO: error handler 
    }
  } else if (ret == SEND_EVENT) {
  }
  return ret;
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_CqService_init(JNIEnv *env, jobject thisObj, jlong eqService) {
  ExternalEqService *externalEqService = *(ExternalEqService**)&eqService;
  assert(externalEqService);
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
}

/*
 * Class:     com_intel_hpnl_core_CqService
 * Method:    free
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_CqService_free(JNIEnv *env, jobject thisObj, jlong cqServicePtr) {
  ExternalCqService *service = *(ExternalCqService**)&cqServicePtr;
  if (service != NULL) {
    delete service;
    service = NULL;
    _set_self(env, thisObj, NULL);
  }
}

