//
// Created by willhua on 2017/3/21.
//
#include "Dehazor.h"
#include "LyhDehazor.h"
#include <android/bitmap.h>


JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_OpenCVMethod_dehazor(JNIEnv *env, jclass cls, jobject bitmap, int width, int height) {
    LyhDehazor *lyhDehazor = new LyhDehazor(width, height, 7);
    LOG("dehazor  begin");
    char * data;

    AndroidBitmap_lockPixels(env, bitmap, (void **)&data);

    lyhDehazor->Dehazor((UCHAR *)data, width, height);
    LOG("dehazor  end");

    AndroidBitmap_unlockPixels(env, bitmap);
}

