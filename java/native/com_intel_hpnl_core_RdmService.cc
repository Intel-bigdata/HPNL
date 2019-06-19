#include "com_intel_hpnl_core_RdmService.h"
#include "demultiplexer/EventType.h"
#include "external_service/ExternalRdmService.cc"

#include <iostream>
static jlong selfPtr;
static jmethodID reallocBufferPool;
static jmethodID regCon;
static jmethodID pushSendBuffer;
static jmethodID pushRecvBuffer;
static jclass parentClass;
static jclass parentConnClass;

static jfieldID _get_self_id(JNIEnv *env, jobject thisObj)
{
  static int init = 0;
  static jfieldID fidSelfPtr;
  if(!init)
  {
    jclass serviceClassTmp;
    serviceClassTmp = env->FindClass("com/intel/hpnl/core/RdmService");

    regCon = (*env).GetMethodID(serviceClassTmp, "regCon", "(JJLjava/lang/String;ILjava/lang/String;IJ)V");

    parentClass = env->FindClass("com/intel/hpnl/core/AbstractService");
    pushSendBuffer = (*env).GetMethodID(parentClass, "pushSendBuffer", "(JI)V");
    pushRecvBuffer = (*env).GetMethodID(parentClass, "pushRecvBuffer", "(JI)V");
    reallocBufferPool = (*env).GetMethodID(parentClass, "reallocBufferPool", "()V");

    parentConnClass = env->FindClass("com/intel/hpnl/api/AbstractConnection");

    fidSelfPtr = env->GetFieldID(serviceClassTmp, "nativeHandle", "J");
    init = 1;
  }
  return fidSelfPtr;
}

static ExternalRdmService*_get_self(JNIEnv *env, jobject thisObj)
{
  jlong selfPtr = env->GetLongField(thisObj, _get_self_id(env, thisObj));
  return *(ExternalRdmService**)&selfPtr;
}

static void _set_self(JNIEnv *env, jobject thisObj, ExternalRdmService *self)
{

  jlong selfPtr = *(jlong*)&self;
  env->SetLongField(thisObj, _get_self_id(env, thisObj), selfPtr);
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmService_init(JNIEnv * env, jobject obj, jint buffer_num, jboolean is_server, jstring prov_name) {
  ExternalRdmService *service = new ExternalRdmService(buffer_num, is_server);
  const char* pname = nullptr;
  if(prov_name != NULL){
  	pname = (*env).GetStringUTFChars(prov_name, 0);
  }
  int res = service->init(pname);
  _set_self(env, obj, service);
  return res;
}

JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_RdmService_listen(JNIEnv *env, jobject obj, jstring ip_, jstring port_, jlong nativeHandle) {
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  const char *ip = (*env).GetStringUTFChars(ip_, 0);
  const char *port = (*env).GetStringUTFChars(port_, 0);
  RdmConnection *con = service->listen(ip, port);
  if (!con) {
//    (*env).CallVoidMethod(obj, reallocBufferPool);
	(*env).CallNonvirtualVoidMethod(obj, parentClass, reallocBufferPool);
    con = (RdmConnection*)service->listen(ip, port);
    if (!con) {
      return -1;
    }
  }

  jlong jcon = *(jlong*)&con;
  jlong jEq = -1;
  jlong id = -1;
  (*env).CallVoidMethod(obj, regCon, jEq, jcon, NULL, 0, NULL, 0, id);

//  (*env).CallVoidMethod(obj, regCon, jcon);

  std::vector<Chunk*> send_buffer = con->get_send_buffer();
  if (!send_buffer.empty()) {
    int chunks_size = send_buffer.size();
    for (int i = 0; i < chunks_size; i++) {
//      (*env).CallVoidMethod(obj, pushSendBuffer, con, send_buffer[i]->buffer_id);
    	(*env).CallNonvirtualVoidMethod(obj, parentClass, pushSendBuffer, jcon, send_buffer[i]->buffer_id);
    }
  }

  std::vector<Chunk*> recv_buffer = con->get_recv_buffer();
  int chunks_size = recv_buffer.size();
  for (int i = 0; i < chunks_size; i++) {
	//	  (*env).CallVoidMethod(obj, pushRecvBuffer, jcon, recv_buffer[i]->buffer_id);
	(*env).CallNonvirtualVoidMethod(obj, parentClass, pushRecvBuffer, jcon, recv_buffer[i]->buffer_id);
  }

  return *(jlong*)&con;
}

JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_RdmService_get_1con(JNIEnv *env, jobject obj, jstring ip_, jstring port_, jlong nativeHandle) {
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  const char *ip = (*env).GetStringUTFChars(ip_, 0);
  const char *port = (*env).GetStringUTFChars(port_, 0);
  RdmConnection *con = (RdmConnection*)service->get_con(ip, port);
  if (!con) {
//    (*env).CallVoidMethod(obj, reallocBufferPool);
	(*env).CallNonvirtualVoidMethod(obj, parentClass, reallocBufferPool);
    con = (RdmConnection*)service->get_con(ip, port);
    if (!con) {
      return -1; 
    }
  }
  char **dest_addr = (char**)malloc(sizeof(char*));
  size_t dest_port;
  char **src_addr = (char**)malloc(sizeof(char*));
  size_t src_port;
  con->get_addr(dest_addr, &dest_port, src_addr, &src_port);
  jstring dest_addr_str = (*env).NewStringUTF(*dest_addr);
  jstring src_addr_str = (*env).NewStringUTF(*src_addr);
  free(dest_addr);
  free(src_addr);

  jlong jcon = *(jlong*)&con;
  jlong jEq = -1;
  jlong id = -1;
  (*env).CallVoidMethod(obj, regCon, jEq, jcon, dest_addr_str, dest_port, src_addr_str, src_port, id);

  std::vector<Chunk*> send_buffer = con->get_send_buffer();
  int chunks_size = send_buffer.size();
  for (int i = 0; i < chunks_size; i++) {
//    (*env).CallVoidMethod(obj, pushSendBuffer, jcon, send_buffer[i]->buffer_id);
	(*env).CallNonvirtualVoidMethod(obj, parentClass, pushSendBuffer, jcon, send_buffer[i]->buffer_id);
  }

  std::vector<Chunk*> recv_buffer = con->get_recv_buffer();
  chunks_size = recv_buffer.size();
  for (int i = 0; i < chunks_size; i++) {
//	  (*env).CallVoidMethod(obj, pushRecvBuffer, jcon, recv_buffer[i]->buffer_id);
	(*env).CallNonvirtualVoidMethod(obj, parentClass, pushRecvBuffer, jcon, recv_buffer[i]->buffer_id);
  }

  return *(jlong*)&con;
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmService_wait_1event(JNIEnv *env, jobject obj, jlong nativeHandle) {
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  Chunk *ck = NULL;
  int block_buffer_size = 0;
  int ret = service->wait_event(&ck, &block_buffer_size);
  if (ret <= 0)
    return ret;
  RdmConnection *con = (RdmConnection*)ck->con;
  if (!con) {
    return -1; 
  }
  jobject javaConn = con->get_java_conn();
  if(!javaConn){
	 return -1;
  }
  jmethodID handleCallback = con->get_java_callback_methodID();
  jint rst = (*env).CallNonvirtualIntMethod(javaConn, parentConnClass, handleCallback, ret, ck->buffer_id, block_buffer_size);
  if (ret == RECV_EVENT && rst) {
	if (con->activate_chunk(ck)) {
	  perror("failed to return receive chunk/buffer");
    }
  }
  return ret;
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmService_remove_1connection(JNIEnv *env, jobject obj, jlong connection_id, jlong nativeHandle){
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  service->reap(connection_id);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmService_set_1recv_1buffer(JNIEnv *env, jobject obj, jobject recv_buffer, jlong size, jint bufferId, jlong nativeHandle) {
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(recv_buffer);
  assert(buffer != NULL);
  service->set_recv_buffer((char*)buffer, size, bufferId);
}

/*
 * Class:     com_intel_hpnl_core_RdmService
 * Method:    set_send_buffer
 * Signature: (Ljava/nio/ByteBuffer;JIJ)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmService_set_1send_1buffer(JNIEnv *env, jobject obj, jobject send_buffer, jlong size, jint bufferId, jlong nativeHandle) {
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(send_buffer);
  assert(buffer != NULL);
  service->set_send_buffer((char*)buffer, size, bufferId);
}

/*
 * Class:     com_intel_hpnl_core_RdmService
 * Method:    free
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmService_free(JNIEnv *env, jobject obj, jlong nativeHandle) {
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  if (service != NULL) {
    delete service;
    service = NULL;
    _set_self(env, obj, NULL);
  }
}
