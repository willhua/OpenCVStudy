#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <time.h>
#include <sys/system_properties.h>
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
    char value[PROP_VALUE_MAX];
    __system_property_get("ro.product.model", value);
    LOG("model:%s" , value);

    FILE *file = fopen("/data/data/com.willhua.opencvstudy/myfile.txt", "w+");
    if(NULL == file)
    {
        LOG("fopen return null");
    }else
    {
        LOG("fopen  not return null");
    }
    int fputs_size = fputs("fputs write\n", file);
    int size_fw = fwrite("frwite", sizeof(char), 5, file);
    char buffer[50];
    fflush(file);
fseek(file, -8, SEEK_CUR);
//fclose(file);
//file = fopen("/data/data/com.willhua.opencvstudy/myfile.txt", "r+t");
    size_t size_fread = fread(buffer, sizeof(char), 6, file);
    LOG("fputs size:%d   %d    %d", fputs_size, size_fw, size_fread);




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