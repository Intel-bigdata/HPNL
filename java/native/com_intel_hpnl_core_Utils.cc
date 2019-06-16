#include "com_intel_hpnl_core_Utils.h"

 #include <sched.h>

 /*
 * Class:     com_intel_hpnl_core_Utils
 * Method:    set_affinity
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Utils_set_1affinity(JNIEnv *env, jclass cls, jlong affinity) {
	int i;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	for(i=0;i<sizeof(affinity)*8;i++){
		if ((affinity >> i) & 1){
			CPU_SET(i, &mask);
		}
	}
	sched_setaffinity(0, sizeof(mask), &mask);
}
