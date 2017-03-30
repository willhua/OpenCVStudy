#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <time.h>
#ifndef JNI_TEST_H
#define JNI_TEST_H

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "lyhopencvn", __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void printfTime()
{
    time_t now;
    struct tm *timenow;
    time(&now);
    timenow = localtime(&now);
    LOG("current time is: %s\n", asctime(timenow) );
}

JNIEXPORT void JNICALL Java_com_willhua_opencvstudy_MainActivity_floatTest(
        JNIEnv *env, jclass obj)
{


    int len = 10000000;
    int *ints = (int*)malloc(sizeof(int) * len);
    for(int i= 0;i<len;++i){
        ints[i] = i%255 + 1;
    }
//////
len-=4;
printfTime();
    for(int i = 0; i < len; i+=4)
    {
        ints[i] = (ints[len - 1 - i] / ints[i] )<<8;
        ints[i+1] = (ints[len - 2 - i] / ints[i+1] )<<8;
        ints[i+2] = (ints[len - 3 - i] / ints[i+2] )<<8;
        ints[i+3] = (ints[len - 4 - i] / ints[i+3] )<<8;
    }
printfTime();
//////
    float fff=255;
    float *floats=(float *)malloc(sizeof(int) * len);
    for(int i= 0;i<len;++i){
        floats[i] = (i%255)/fff;
    }
    //////
    printfTime();
    for(int i = 0; i < len; ++i)
    {
        floats[i] = ints[i] / floats[i];
    }
printfTime();
    //////
    delete [] ints;
    delete [] floats;
}

#ifdef __cplusplus
}
#endif

#endif //JNI_TEST_H