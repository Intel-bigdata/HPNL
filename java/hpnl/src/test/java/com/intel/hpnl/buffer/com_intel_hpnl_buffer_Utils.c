#include <jni.h>
#include "com_intel_hpnl_buffer_Utils.h"
#include <assert.h>

JNIEXPORT void JNICALL Java_com_intel_hpnl_buffer_Utils_verify
  (JNIEnv * env, jclass class, jlong addr){
	printf("addr: %ld\n", addr);
	char *buf = (char *)addr;
	assert(buf[0] == 'b');
	assert(buf[1] == 'c');
	assert(buf[2] == 'd');	
	int size = 10*1024*1024;
	assert(buf[size-1] == 'k');
	assert(buf[size-2] == 'j');
	assert(buf[size-3] == 'i');
	printf("verified\n");	
} 
