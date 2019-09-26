#include "com_intel_hpnl_core_RdmConnection.h"
#include "core/RdmConnection.h"
#include <iostream>

static jclass parentClass;
static jmethodID handleCallback;

static jfieldID _get_self_id(JNIEnv *env, jobject thisObj)
{
  static int init = 0;
  static jfieldID fidSelfPtr;
  if (!init) {
    jclass thisClass = env->GetObjectClass(thisObj);
    fidSelfPtr = env->GetFieldID(thisClass, "nativeHandle", "J");

    parentClass = env->FindClass("com/intel/hpnl/api/AbstractConnection");
    handleCallback = (*env).GetMethodID(parentClass, "handleCallback", "(III)I");
    init = 1;
  }
  return fidSelfPtr;
}

static void _set_self(JNIEnv *env, jobject thisObj, long nativeCon)
{
  env->SetLongField(thisObj, _get_self_id(env, thisObj), nativeCon);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmConnection_init(JNIEnv *env, jobject obj, jlong nativeHandle) {
	RdmConnection *fiConn = *(RdmConnection**)&nativeHandle;
	jobject javaConn = env->NewGlobalRef(obj);
	fiConn->set_java_conn(javaConn);
	fiConn->set_java_callback_methodID(handleCallback);
    _set_self(env, obj, nativeHandle);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmConnection_get_1local_1name(JNIEnv *env, jobject obj, jobject buffer, jlong nativeHandle) {
  RdmConnection *con = *(RdmConnection**)&nativeHandle;
  jbyte* bytes = (jbyte*)(*env).GetDirectBufferAddress(buffer);
  char* local_name = con->get_local_name();
  memcpy(bytes, local_name, con->get_local_name_length());
}

/*
 * Class:     com_intel_hpnl_core_RdmConnection
 * Method:    get_local_name_length
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_get_1local_1name_1length(JNIEnv *env, jobject obj, jlong nativeHandle) {
  RdmConnection *con = *(RdmConnection**)&nativeHandle;
  return con->get_local_name_length();
}

JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_RdmConnection_get_1connection_1id(JNIEnv *env, jobject obj, jlong nativeHandle){
  RdmConnection *con = *(RdmConnection**)&nativeHandle;
  return con->get_id();
}

/*
 * Class:     com_intel_hpnl_core_RdmConnection
 * Method:    send
 * Signature: (IIJ)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_send(JNIEnv *env, jobject obj, jint blockBufferSize, jint bufferId, jlong nativeHandle) {
  RdmConnection *con = *(RdmConnection**)&nativeHandle;
  int res = con->send(blockBufferSize, bufferId);
  return res;
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_sendRequest(JNIEnv *env, jobject obj, jint blockBufferSize, jint bufferId, jlong nativeHandle) {
  RdmConnection *con = *(RdmConnection**)&nativeHandle;
  int res = con->sendRequest(blockBufferSize, bufferId);
  return res;
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_sendTo(JNIEnv *env, jobject obj, jint blockBufferSize, jint bufferId, jlong peerAddress, jlong nativeHandle) {
  RdmConnection *con = *(RdmConnection**)&nativeHandle;
  int res = con->sendTo(blockBufferSize, bufferId, (uint64_t)peerAddress);
  return res;
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_sendBuf(JNIEnv *env, jobject obj, jobject buffer, jint bufferId, jint ctxId, jint bufferSize, jlong nativeHandle) {
  RdmConnection *con = *(RdmConnection**)&nativeHandle;
  jbyte* bytes = (jbyte*)(*env).GetDirectBufferAddress(buffer);
  int res = con->sendBuf((char*)bytes, bufferId, ctxId, bufferSize);
  return res;
}

/*
 * Class:     com_intel_hpnl_core_RdmConnection
 * Method:    sendBufTo
 * Signature: (Ljava/nio/ByteBuffer;I[BJ)I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmConnection_sendBufTo(JNIEnv *env, jobject obj, jobject buffer, jint bufferId, jint ctxId, jint bufferSize, jlong peerAddress, jlong nativeHandle) {
  RdmConnection *con = *(RdmConnection**)&nativeHandle;
  jbyte* bytes = (jbyte*)(*env).GetDirectBufferAddress(buffer);
  int res = con->sendBufTo((char*)bytes, bufferId, ctxId, bufferSize, (uint64_t)peerAddress);
  return res;
}

JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_RdmConnection_resolve_1peer_1name(JNIEnv *env, jobject obj, jobject peerName, jlong nativeHandle){
	RdmConnection *con = *(RdmConnection**)&nativeHandle;
	jbyte* name = (jbyte*)(*env).GetDirectBufferAddress(peerName);
	return con->resolve_peer_name((char*)name);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmConnection_releaseRecvBuffer(JNIEnv *env, jobject thisObj, jint rdmaBufferId, jlong conPtr){
	RdmConnection *con = *(RdmConnection**)&conPtr;
	con->activate_chunk(rdmaBufferId);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmConnection_adjustSendTarget(JNIEnv *env, jobject thisObj, jint sendCtxId, jlong conPtr){
	RdmConnection *con = *(RdmConnection**)&conPtr;
	con->adjust_send_target(sendCtxId);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmConnection_deleteGlobalRef
  (JNIEnv *env, jobject thisObj, jlong conPtr){
	RdmConnection *con = *(RdmConnection**)&conPtr;
	env->DeleteGlobalRef(con->get_java_conn());
	con->set_java_conn(NULL);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmConnection_free
  (JNIEnv *env, jobject thisObj, jlong conPtr){
	RdmConnection *con = *(RdmConnection**)&conPtr;
	if(con != NULL){
		delete con;
		con = NULL;
	}
}
