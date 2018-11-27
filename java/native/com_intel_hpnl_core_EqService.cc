#include <rdma/fi_domain.h>

#include "HPNL/ExternalEqService.h"
#include "HPNL/Connection.h"

#include "com_intel_hpnl_core_EqService.h"

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

static ExternalEqService*_get_self(JNIEnv *env, jobject thisObj)
{
  jlong selfPtr = env->GetLongField(thisObj, _get_self_id(env, thisObj));
  return *(ExternalEqService**)&selfPtr;
}

static void _set_self(JNIEnv *env, jobject thisObj, ExternalEqService *self)
{
  jlong selfPtr = *(jlong*)&self;
  env->SetLongField(thisObj, _get_self_id(env, thisObj), selfPtr);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_init(JNIEnv *env, jobject thisObj, jstring ip_, jstring port_, jint buffer_num_, jboolean is_server_) {
  const char *ip = (*env).GetStringUTFChars(ip_, 0);
  const char *port = (*env).GetStringUTFChars(port_, 0);
  const bool is_server = (bool)is_server_;
  ExternalEqService *service = new ExternalEqService(ip, port, buffer_num_, is_server);
  _set_self(env, thisObj, service);
}

/*
 * Class:     com_intel_hpnl_EqService
 * Method:    finalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_finalize(JNIEnv *env, jobject thisObj) {
  _set_self(env, thisObj, NULL);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_free(JNIEnv *env, jobject thisObj) {
  ExternalEqService *service = _get_self(env, thisObj);
  if (service != NULL) {
    delete service;
    _set_self(env, thisObj, NULL);
  }
}


/*
 * Class:     com_intel_hpnl_EqService
 * Method:    connect
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_EqService_connect(JNIEnv *env, jobject thisObj) {
  ExternalEqService *service = _get_self(env, thisObj);
  fid_eq *eq = service->connect();
  jlong ret = *(jlong*)&eq;
  return ret;
}

/*
 * Class:     com_intel_hpnl_EqService
 * Method:    wait_eq_event
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_EqService_wait_1eq_1event(JNIEnv *env, jobject thisObj) {
  ExternalEqService *service = _get_self(env, thisObj);
  fi_info *info = NULL;
  fid_eq *eq;
  int ret = service->wait_eq_event(&info, &eq);
  if (ret < 0) return ret;
  FIConnection *con = (FIConnection*)service->get_connection(eq);
  jclass thisClass = (*env).GetObjectClass(thisObj);
  if (ret == ACCEPT_EVENT) {
    //accept new connection and register eq id
    fid_eq *new_eq;
    new_eq = service->accept(info);
    service->add_eq_event(new_eq);
  } else if (ret == CONNECTED_EVENT) {
    //register connection  
    jmethodID registerCon = (*env).GetMethodID(thisClass, "registerCon", "(JJ)V");
    assert(registerCon);
    jlong jEq = *(jlong*)&eq;
    jlong jCon = *(jlong*)&con;
    (*env).CallVoidMethod(thisObj, registerCon, jEq, jCon);

    //set send buffer;
    std::vector<Chunk*> send_buffer = con->get_send_buffer();
    int chunks_size = send_buffer.size();
    for (int i = 0; i < chunks_size; i++) {
      jmethodID putSendBuffer = (*env).GetMethodID(thisClass, "putSendBuffer", "(JI)V");
      assert(putSendBuffer);
      (*env).CallVoidMethod(thisObj, putSendBuffer, jEq, send_buffer[i]->rdma_buffer_id);
    }
    
    //callback
    jmethodID handleEqCallback = (*env).GetMethodID(thisClass, "handleEqCallback", "(JII)V");
    (*env).CallVoidMethod(thisObj, handleEqCallback, jEq, ret, 0);
    {
      std::lock_guard<std::mutex> l(con->con_mtx);
      con->status = CONNECTED;
    }
    con->con_cv.notify_one();
  } else if (ret == SHUTDOWN) {
    jlong jEq = *(jlong*)&eq;
    jmethodID deregCon = (*env).GetMethodID(thisClass, "deregCon", "(J)V");
    assert(deregCon);
    (*env).CallVoidMethod(thisObj, deregCon, jEq);
  } else {
  }
  return ret;
}

/*
 * Class:     com_intel_hpnl_core_EqService
 * Method:    add_eq_event
 * Signature: (j)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_EqService_add_1eq_1event(JNIEnv *env, jobject thisObj, jlong eqPtr) {
  ExternalEqService *service = _get_self(env, thisObj);
  fid_eq *eq = *(fid_eq**)&eqPtr;
  return service->add_eq_event(eq);
}

/*
 * Class:     com_intel_hpnl_core_EqService
 * Method:    delete_eq_event
 * Signature: (j)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_EqService_delete_1eq_1event(JNIEnv *env, jobject thisObj, jlong eqPtr) {
  ExternalEqService *service = _get_self(env, thisObj);
  if (service == NULL) return 0;
  fid_eq *eq = *(fid_eq**)&eqPtr;
  return service->delete_eq_event(eq);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_shutdown(JNIEnv *env, jobject thisObj, jlong eqPtr) {
  ExternalEqService *service = _get_self(env, thisObj);
  fid_eq *eq = *(fid_eq**)&eqPtr;
  FIConnection *con = (FIConnection*)service->get_connection(eq);

  if (con->status < DOWN) {
    con->shutdown();
    con->status = DOWN;
    service->reap(&eq->fid);
  }
}


/*
 * Class:     com_intel_hpnl_EqService
 * Method:    set_recv_buffer
 * Signature: (Ljava/nio/ByteBuffer;J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_set_1recv_1buffer(JNIEnv *env, jobject thisObj, jobject recv_buffer, jlong size, jint rdmaBufferId) {
  ExternalEqService *service = _get_self(env, thisObj);
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(recv_buffer);
  assert(buffer != NULL);
  service->set_recv_buffer((char*)buffer, size, rdmaBufferId);
}

/*
 * Class:     com_intel_hpnl_EqService
 * Method:    set_send_buffer
 * Signature: (Ljava/nio/ByteBuffer;J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_set_1send_1buffer(JNIEnv *env, jobject thisObj, jobject send_buffer, jlong size, jint rdmaBufferId) {
  ExternalEqService *service = _get_self(env, thisObj);
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(send_buffer);
  service->set_send_buffer((char*)buffer, size, rdmaBufferId);
}

/*
 * Class:     com_intel_hpnl_EqService
 * Method:    reg_rma_buffer
 * Signature: (Ljava/nio/ByteBuffer;J)V
 */
JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_EqService_reg_1rma_1buffer(JNIEnv *env, jobject thisObj, jobject send_buffer, jlong size, jint rdmaBufferId) {
  ExternalEqService *service = _get_self(env, thisObj);
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(send_buffer);
  return service->reg_rma_buffer((char*)buffer, size, rdmaBufferId);
}

JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_EqService_reg_1rma_1buffer_1by_1address(JNIEnv *env, jobject thisObj, jlong address, jlong size, jint rdmaBufferId) {
  ExternalEqService *service = _get_self(env, thisObj);
  return service->reg_rma_buffer(*(char**)&address, size, rdmaBufferId);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_EqService_unreg_1rma_1buffer(JNIEnv *env, jobject thisObj, jint rdmaBufferId) {
  ExternalEqService *service = _get_self(env, thisObj);
  service->unreg_rma_buffer(rdmaBufferId);
}

/*
 * Class:     com_intel_hpnl_core_EqService
 * Method:    get_buffer_address
 * Signature: (Ljava/nio/ByteBuffer)J
 */
JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_EqService_get_1buffer_1address(JNIEnv *env, jobject thisObj, jobject rma_buffer) {
  ExternalEqService *service = _get_self(env, thisObj);
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(rma_buffer);
  jlong buffer_address = *(jlong*)&buffer;
  return buffer_address;
}

