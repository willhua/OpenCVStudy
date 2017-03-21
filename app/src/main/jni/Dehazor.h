//
// Created by willhua on 2017/3/21.
//

#include <jni.h>
#include <android/log.h>

#ifndef OPENCVSTUDY_DEHAZOR_H
#define OPENCVSTUDY_DEHAZOR_H

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "lyhopencvn", __VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif

void dehazor(uchar *rgba, int width, int height);
uchar * boxfilter(uchar *data, int width, int height);


#ifdef __cplusplus
}
#endif  //__cpp
}


#endif //OPENCVSTUDY_DEHAZOR_H
