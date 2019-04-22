#include "com_intel_hpnl_core_Connection.h"

#include "HPNL/Connection.h"
#include "HPNL/FIStack.h"

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

static Connection*_get_self(JNIEnv *env, jobject thisObj)
{
  jlong selfPtr = env->GetLongField(thisObj, _get_self_id(env, thisObj));
  return *(Connection**)&selfPtr;
}

static void _set_self(JNIEnv *env, jobject thisObj, long nativeCon)
{
  env->SetLongField(thisObj, _get_self_id(env, thisObj), nativeCon);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_recv(JNIEnv *env, jobject thisObj, jobject bufferObj, jint mid, jlong conPtr) {
  Connection *con = *(Connection**)&conPtr;
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(bufferObj);
  con->recv((char*)buffer, mid);
}

JNIEXPORT int JNICALL Java_com_intel_hpnl_core_Connection_send(JNIEnv *env, jobject thisObj, jint blockBufferSize, jint rdmaBufferId, jlong conPtr) {
  Connection *con = *(Connection**)&conPtr;
  return con->send(blockBufferSize, rdmaBufferId);
}

JNIEXPORT int JNICALL Java_com_intel_hpnl_core_Connection_read(JNIEnv *env, jobject thisObj, jint rdmaBufferId, jint localOffset, jlong len, jlong remoteAddr, jlong remoteMr, jlong conPtr) {
  Connection *con = *(Connection**)&conPtr;
  return con->read(rdmaBufferId, localOffset, len, remoteAddr, remoteMr);
}

/*
 * Class:     com_intel_hpnl_Connection
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_init(JNIEnv *env, jobject thisObj, jlong nativeCon) {
  _set_self(env, thisObj, nativeCon);
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_Connection_get_1cq_1index
  (JNIEnv *env, jobject thisObj){
  Connection *con = _get_self(env, thisObj);
  return con->get_cq_index();
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_releaseRecvBuffer(JNIEnv *env, jobject thisObj, jint rdmaBufferId){
	Connection *con = _get_self(env, thisObj);
	con->activate_chunk(rdmaBufferId);
}

/*
 * Class:     com_intel_hpnl_Connection
 * Method:    finalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_finalize(JNIEnv *env, jobject thisObj) {
  _set_self(env, thisObj, 0);
}
