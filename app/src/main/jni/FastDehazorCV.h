//
// Created by lisan on 2017/4/11.
//

#pragma once

#include "JniEnvInit.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/types_c.h>

#define INPUT_NULL -1
#define MAX_P 0.9f

//to realizing the fast-dehazor by opencv
class FastDehazorCV
{
public:
    FastDehazorCV();
    ~FastDehazorCV();
    int process(unsigned char * rgba, int width, int height, int boxRadius);
    void setP(float p);
    static JavaVM * mVM;
protected:
private:
    static void * getDarkThread(void * args);
    unsigned char getDarkChannel(unsigned char * rgba, unsigned char * out, int width, int height);

    int * mDivN;
    float mP;
    int mSkyThreshold;
    unsigned char * mResultTable;
    void InitResultTable();
    //在算法中，air的三个通道的值都是一样的
    unsigned char mAir;
    void BoxDivN(int *out, int width, int height, int r);
    template<class T1, class T2> void BoxFilter(T1 *data, T2 *outdata, int r, int width, int height, T2 t);
};

