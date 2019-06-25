#include <jni.h>
/* Header for class com_intel_hpnl_core_Utils */

#ifndef _Included_com_intel_hpnl_core_Utils
#define _Included_com_intel_hpnl_core_Utils
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_intel_hpnl_core_Utils
 * Method:    set_affinity
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_intel_hpnl_core_Utils_set_1affinity
  (JNIEnv *, jclass, jlong);

/*
 * Class:     com_intel_hpnl_core_Utils
 * Method:    get_pid
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_intel_hpnl_core_Utils_get_1pid
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
