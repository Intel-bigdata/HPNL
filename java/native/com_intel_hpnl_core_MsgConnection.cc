#include "com_intel_hpnl_core_MsgConnection.h"

#include "HPNL/Connection.h"
#include "core/MsgStack.h"
#include "core/MsgConnection.h"

static jmethodID handleCallback;

static jfieldID _get_self_id(JNIEnv *env, jobject thisObj)
{
  static int init = 0;
  static jfieldID fidSelfPtr;
  if (!init) {
    jclass thisClass = env->GetObjectClass(thisObj);
    fidSelfPtr = env->GetFieldID(thisClass, "nativeHandle", "J");

    jclass connClass = env->FindClass("com/intel/hpnl/core/MsgConnection");
    handleCallback = (*env).GetMethodID(connClass, "handleCallback", "(III)I");
    init = 1;
  }
  return fidSelfPtr;
}

static void _set_self(JNIEnv *env, jobject thisObj, long nativeCon)
{
  env->SetLongField(thisObj, _get_self_id(env, thisObj), nativeCon);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_MsgConnection_recv(JNIEnv *env, jobject thisObj, jobject bufferObj, jint mid, jlong conPtr) {
  Connection *con = *(Connection**)&conPtr;
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(bufferObj);
  con->recv((char*)buffer, mid);
}

JNIEXPORT int JNICALL Java_com_intel_hpnl_core_MsgConnection_send(JNIEnv *env, jobject thisObj, jint blockBufferSize, jint rdmaBufferId, jlong conPtr) {
  Connection *con = *(Connection**)&conPtr;
  return con->send(blockBufferSize, rdmaBufferId);
}

JNIEXPORT int JNICALL Java_com_intel_hpnl_core_MsgConnection_read(JNIEnv *env, jobject thisObj, jint rdmaBufferId, jint localOffset, jlong len, jlong remoteAddr, jlong remoteMr, jlong conPtr) {
  Connection *con = *(Connection**)&conPtr;
  return con->read(rdmaBufferId, localOffset, len, remoteAddr, remoteMr);
}

/*
 * Class:     com_intel_hpnl_MsgConnection
 * Method:    init
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_MsgConnection_init(JNIEnv *env, jobject thisObj, jlong nativeCon) {
  MsgConnection *fiConn = *(MsgConnection**)&nativeCon;
  jobject javaConn = env->NewGlobalRef(thisObj);
  fiConn->set_java_conn(javaConn);
  fiConn->set_java_callback_methodID(handleCallback);
  _set_self(env, thisObj, nativeCon);
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_MsgConnection_get_1cq_1index
  (JNIEnv *env, jobject thisObj, jlong conPtr){
  Connection *con = *(Connection**)&conPtr;
  return con->get_cq_index();
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_MsgConnection_releaseRecvBuffer(JNIEnv *env, jobject thisObj, jint rdmaBufferId, jlong conPtr){
	Connection *con = *(Connection**)&conPtr;
	con->activate_chunk(rdmaBufferId);
}

/*
 * Class:     com_intel_hpnl_MsgConnection
 * Method:    finalize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_MsgConnection_finalize(JNIEnv *env, jobject thisObj) {}

/*
 * Class:     com_intel_hpnl_core_MsgConnection
 * Method:    deleteGlobalRef
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_MsgConnection_deleteGlobalRef
  (JNIEnv *env, jobject thisObj, jlong conPtr){
	MsgConnection *con = *(MsgConnection**)&conPtr;
	env->DeleteGlobalRef(con->get_java_conn());
	con->set_java_conn(NULL);
}

/*
 * Class:     com_intel_hpnl_core_MsgConnection
 * Method:    free
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_MsgConnection_free
  (JNIEnv *env, jobject thisObj, jlong conPtr){
	MsgConnection *con = *(MsgConnection**)&conPtr;
	if(con != NULL){
		delete con;
		con = NULL;
	}
}
