//
// Created by lisan on 2017/4/13.
//

#ifndef OPENCVSTUDY_MYJNI_H
#define OPENCVSTUDY_MYJNI_H


#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <time.h>
#define MINT(a,b,c) ((a)<(b)?((a)<(c)?(a):(c)):((b)<(c)?(b):(c)))
#define MUL(a,b,c) (a[c] * b[c])
#define CLAM(a) ((a) > 255 ? 255 : ((a) < 0 ? 0 : (a)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) > 0 ? (a) : (-a))

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "MyJni", __VA_ARGS__)

class JniEnvInit{
public:
    static JavaVM * gVM;
};


#endif //OPENCVSTUDY_MYJNI_H
