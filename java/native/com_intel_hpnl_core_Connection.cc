#include "com_intel_hpnl_core_Connection.h"

#include "HPNL/Connection.h"
#include "HPNL/FIStack.h"

static jfieldID _get_self_id(JNIEnv *env, jobject thisObj)
{
  static int init = 0;
  static jfieldID fidSelfPtr;
  if(!init)
  {
    jclass thisClass = env->GetObjectClass(thisObj);
    fidSelfPtr = env->GetFieldID(thisClass, "nativeHandle", "J");
  }
  return fidSelfPtr;
}

static  Connection*_get_self(JNIEnv *env, jobject thisObj)
{
  jlong selfPtr = env->GetLongField(thisObj, _get_self_id(env, thisObj));
  return *(Connection**)&selfPtr;
}

static void _set_self(JNIEnv *env, jobject thisObj, long nativeCon)
{
  env->SetLongField(thisObj, _get_self_id(env, thisObj), nativeCon);
}

/*
 * Class:     com_intel_hpnl_Connection
 * Method:    read
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_read(JNIEnv *env, jobject thisObj, jobject bufferObj, jint mid) {
  Connection *con = _get_self(env, thisObj);
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(bufferObj);
  con->read((char*)buffer, mid);
}

/*
 * Class:     com_intel_hpnl_Connection
 * Method:    write
 * Signature: (Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_write(JNIEnv *env, jobject thisObj, jstring jstr, jint mid_1, jint mid_2) {
  Connection *con = _get_self(env, thisObj); 
  const char *str = env->GetStringUTFChars(jstr, 0);
  con->write(str, BUFFER_SIZE, mid_2);
}

/*
 * Class:     com_intel_hpnl_Connection
 * Method:    shutdown
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Connection_shutdown(JNIEnv *env, jobject thisObj) {
  Connection *con = _get_self(env, thisObj); 
  assert(con);
  con->shutdown();
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
  //Connection *con = _get_self(env, thisObj);
  //if (con != NULL) {
  //  _set_self(env, thisObj, 0);
  //}
}
