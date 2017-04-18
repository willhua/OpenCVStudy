//
// Created by willhua on 2017/3/21.
//



#ifndef OPENCVSTUDY_DEHAZOR_H
#define OPENCVSTUDY_DEHAZOR_H

#include "JniEnvInit.h"
#include "FastDehazorCV.h"

class Dehazor{
public:
static FastDehazorCV * fastDehazorCV;
};



#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_dehazor(JNIEnv *env, jclass cls, jobject bitmap, int width, int height);
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_fastDehazor(JNIEnv *env, jclass cls, jobject bitmap, int width, int height);
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_fastDehazorCV(JNIEnv *env, jclass cls, jobject bitmap, int width, int height, int radius, float p);
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_getDark(JNIEnv *env, jclass cls, jobject bitmap, int width, int height);
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_initNative(JNIEnv *env, jclass cls);
JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_freeNative(JNIEnv *env, jclass cls);

#ifdef __cplusplus
}
#endif  //__cpp



#endif //OPENCVSTUDY_DEHAZOR_H
