//
// Created by lisan on 2017/4/11.
//

#pragma once

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/types_c.h>

#define INPUT_NULL -1
#define MAX_P 0.9f
#define MINT(a,b,c) ((a)<(b)?((a)<(c)?(a):(c)):((b)<(c)?(b):(c)))
#define CLAM(a) ((a) > 255 ? 255 : ((a) < 0 ? 0 : (a)))

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "MyJni", __VA_ARGS__)


//to realizing the fast-dehazor by opencv
class FastDehazorCV
{
public:
    FastDehazorCV(int w, int h);
    ~FastDehazorCV();
    int process(unsigned char * rgba, int width, int height, int boxRadius, float p, int t);
    static int mThreshold;
private:
    unsigned char * mLx;
    unsigned char * mDarkChannel;
    unsigned char getDarkChannel(unsigned char * rgba, unsigned char * out, int width, int height, unsigned char &max);
    int mRadius;
    float mP;
    unsigned char * mResultTable;
    void InitResultTable();
    //在算法中，air的三个通道的值都是一样的
    unsigned char mAir;
};

