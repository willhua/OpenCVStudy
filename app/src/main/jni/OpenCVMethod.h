//
// Created by willhua on 2017/3/11.
//

#ifndef OPENCVSTUDY_OPENCVMETHOD_H
#define OPENCVSTUDY_OPENCVMETHOD_H

#include "JniEnvInit.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jintArray JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_gray(
        JNIEnv *env, jclass obj, jintArray buf, int w, int h);

#ifdef __cplusplus
}
#endif

#endif //OPENCVSTUDY_OPENCVMETHOD_H
