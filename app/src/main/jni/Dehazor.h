//
// Created by willhua on 2017/3/21.
//



#ifndef OPENCVSTUDY_DEHAZOR_H
#define OPENCVSTUDY_DEHAZOR_H

#include "JniEnvInit.h"

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_dehazor(JNIEnv *env, jclass cls, jobject bitmap, int width, int height);
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_fastDehazor(JNIEnv *env, jclass cls, jobject bitmap, int width, int height);
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_fastDehazorCV(JNIEnv *env, jclass cls, jobject bitmap, int width, int height, int radius);
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_getDark(JNIEnv *env, jclass cls, jobject bitmap, int width, int height);

#ifdef __cplusplus
}
#endif  //__cpp



#endif //OPENCVSTUDY_DEHAZOR_H
