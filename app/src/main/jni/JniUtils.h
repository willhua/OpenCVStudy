//
// Created by willhua on 2017-4-14.
//

#ifndef OPENCVSTUDY_JNIUTILS_H
#define OPENCVSTUDY_JNIUTILS_H


#include "JniEnvInit.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

JNIEXPORT void JNICALL JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_getDivN(JNIEnv * env, jclass clazz, jintArray array);


#endif //OPENCVSTUDY_JNIUTILS_H
