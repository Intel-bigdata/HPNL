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

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmService_init(JNIEnv * env, jobject obj,
		jint buffer_num, jint recv_buffer_num, jint ctx_num, jint endpoint_num,
		jint read_batch_size, jboolean is_server, jstring prov_name) {
  ExternalRdmService *service = new ExternalRdmService(buffer_num, recv_buffer_num, ctx_num,
		  endpoint_num, read_batch_size, is_server);
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
  if((*env).ExceptionOccurred()){
  	  return -1;
  }
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

JNIEXPORT jlong JNICALL Java_com_intel_hpnl_core_RdmService_get_1con(JNIEnv *env, jobject obj, jstring ip_, jstring port_,
		jlong src_provider_addr, jint cq_index, jlong nativeHandle) {
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  const char *ip = (*env).GetStringUTFChars(ip_, 0);
  const char *port = (*env).GetStringUTFChars(port_, 0);
  RdmConnection *con = (RdmConnection*)service->get_con(ip, port, src_provider_addr, cq_index);
  if (!con) {
//    (*env).CallVoidMethod(obj, reallocBufferPool);
	(*env).CallNonvirtualVoidMethod(obj, parentClass, reallocBufferPool);
    con = (RdmConnection*)service->get_con(ip, port, src_provider_addr, cq_index);
    if (!con) {
      return -1; 
    }
  }

  jlong jcon = *(jlong*)&con;
  jlong jEq = -1;
  jlong id = -1;
  (*env).CallVoidMethod(obj, regCon, jEq, jcon, NULL, 0, NULL, 0, id);
  if((*env).ExceptionOccurred()){
	  (*env).ExceptionDescribe();
	  return -1;
  }

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

int process_event(JNIEnv *env, Chunk *ck, int buffer_id, int buffer_size, int event_type){
	std::cout<<"event type: "<<event_type<<std::endl;
  buffer_id = ck->buffer_id;
  RdmConnection *con = (RdmConnection*)ck->con;
  if(ck->ctx_id < 0){
	  delete ck;
  }
  jobject javaConn = con->get_java_conn();
  jmethodID handleCallback = con->get_java_callback_methodID();
  jint rst = (*env).CallNonvirtualIntMethod(javaConn, parentConnClass, handleCallback, event_type, buffer_id, buffer_size);
  if((*env).ExceptionOccurred()){
	  (*env).ExceptionDescribe();
	  return -1;
  }
  if (event_type == RECV_EVENT && rst) {
	if (con->activate_chunk(ck)) {
	  perror("failed to return receive chunk/buffer");
	}
  }
  return 0;
}

JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_RdmService_wait_1event(JNIEnv *env, jobject obj, jint cq_index, jlong nativeHandle) {
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  int ret = service->wait_event(env, cq_index, &process_event);
  return ret;
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmService_remove_1connection(JNIEnv *env, jobject obj, jlong connection_id, jlong nativeHandle){
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  service->reap(connection_id);
}

JNIEXPORT void JNICALL Java_com_intel_hpnl_core_RdmService_set_1recv_1buffer(JNIEnv *env, jobject obj, jobject recv_buffer, jlong size, jint bufferId, jlong nativeHandle) {
  ExternalRdmService *service = *(ExternalRdmService**)&nativeHandle;
  jbyte* buffer = (jbyte*)(*env).GetDirectBufferAddress(recv_buffer);
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
