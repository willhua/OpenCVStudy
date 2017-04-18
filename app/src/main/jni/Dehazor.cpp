//
// Created by willhua on 2017/3/21.
//
#include "Dehazor.h"
#include "LyhDehazor.h"
#include "FastDehazor.h"
#include "FastDehazorCV.h"
#include <android/bitmap.h>


FastDehazorCV * Dehazor::fastDehazorCV = NULL;

JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_dehazor(JNIEnv *env, jclass cls, jobject bitmap, int width, int height) {
    LyhDehazor *lyhDehazor = new LyhDehazor(width, height, 7);
    LOG("dehazor  begin");
    char * data;

    AndroidBitmap_lockPixels(env, bitmap, (void **)&data);

    lyhDehazor->Dehazor((UCHAR *)data, width, height);
    LOG("dehazor  end");

    AndroidBitmap_unlockPixels(env, bitmap);
}


JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_fastDehazor(JNIEnv *env, jclass cls, jobject bitmap, int width, int height) {
    LOG("fast  dehazor  begin");
    FastDehazor *fastDehazor = new FastDehazor(width, height);
    char * data;

    AndroidBitmap_lockPixels(env, bitmap, (void **)&data);
    fastDehazor->process((UCHAR *)data, width, height);

    AndroidBitmap_unlockPixels(env, bitmap);
    LOG("fast   dehazor  end");
}

JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_fastDehazorCV(JNIEnv *env, jclass cls, jobject bitmap, int width, int height, int radius, float p) {
    LOG("fast  dehazor  begin");
    char * data;

    AndroidBitmap_lockPixels(env, bitmap, (void **)&data);
    Dehazor::fastDehazorCV->process((UCHAR *)data, width, height, radius, p);

    AndroidBitmap_unlockPixels(env, bitmap);
    LOG("fast   dehazor  end");
}

JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_initNative(JNIEnv *env, jclass cls) {
    LOG("fast  dehazor  init");
    Dehazor::fastDehazorCV = new FastDehazorCV();
}

JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_freeNative(JNIEnv *env, jclass cls) {
    if(Dehazor::fastDehazorCV != NULL)
    {
        delete Dehazor::fastDehazorCV;
        LOG("fast   dehazor  free");
    }
}

JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_getDark(JNIEnv *env, jclass cls, jobject bitmap, int width, int height) {
    LOG("fast  dehazor  begin");
    char * data;
    UCHAR * out = (UCHAR *)malloc(sizeof(UCHAR) * width * height);
    AndroidBitmap_lockPixels(env, bitmap, (void **)&data);
  //  fastDehazor->process((UCHAR *)data, width, height);

    AndroidBitmap_unlockPixels(env, bitmap);
    LOG("fast   dehazor  end");
}

