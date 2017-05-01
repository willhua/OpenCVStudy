//
// Created by lisan on 2017/4/11.
//

#include "FastDehazorCV.h"
#include <pthread.h>

using namespace cv;

int FastDehazorCV::mThreshold = 60;

FastDehazorCV::FastDehazorCV()
{
    mP = 1.3f;
    mThreshold = 60;
    mRadius = 50;
    mSkyThreshold = 15;
    mResultTable = (unsigned char *)malloc(sizeof(unsigned char) * 256 * 256);
}

FastDehazorCV::~FastDehazorCV()
{
    if(mResultTable != NULL)
    {
        free(mResultTable);
    }
}

void FastDehazorCV::InitResultTable()
{
    const float Threshold = 50;
    const float s = 1 / Threshold;
    float scale = 1.0;
    float air = 1.0f / mAir;
    int index = 0;
    float result;
    unsigned char v;
    for(int i = 0; i < 256; ++i) //符合暗通道的区域
    {
        for(int j = 0; j < 256; ++j)
        {
            result = (i - j) / (1 - j * air);
            mResultTable[index++] = (unsigned char)CLAM(result);
        }
    }
}



typedef struct TaskParam
{
    unsigned char * rgba;
    unsigned char * out;
    int start;  //以像素为单位的，即对于out可以直接使用，但是对于rgba则需要乘以4
    int end;
    unsigned char * dark;
    float p;
};




inline void findMaxMin(unsigned char a, unsigned char b, unsigned char c, unsigned char &max,
                       unsigned char &min)
{
    if(a > b)
    {
        if(b > c)
        {
            if(a > max) max = a;
            min = c;
        }
        else
        {
            min = b;
            if(a > c && a > max)
            {
                max = a;
            }
            else if(c > max)
            {
                max = c;
            }
        }
    }
    else
    {
        if(a > c)
        {
            if(b > max) max = b;
            min = c;
        }
        else
        {
            min = a;
            if(b > c && b > max)
            {
                max = b;
            }
            else if(c > max)
            {
                max = c;
            }
        }
    }
}


void * FastDehazorCV::getDarkThread(void * args)
{
    long sum1 = 0;
    long sum2 = 0;
    unsigned char max1;
    unsigned char max2;
    //LOG("gvm %d", JniEnvInit::gVM);
    TaskParam * param = (TaskParam *)args;
    int end  = param->end;
    unsigned char * out = param->out;
    unsigned char * rgba = param->rgba;
    unsigned char min1;
    unsigned char min2;
    //LOG("getDarkThread rgba%d, out%d, start%d, end%d", rgba, out, param->start, end);
    for(int i = param->start, j = (i << 2); i < end; i+=2, j+=8)
    {
        findMaxMin(rgba[j], rgba[j+1], rgba[j+2], max1, min1);
        findMaxMin(rgba[j+4], rgba[j+5], rgba[j+5], max2, min2);
        //dark1 = MINT(rgba[j], rgba[1+j], rgba[2+j]);
        //dark1 = MINT(rgba[j+4], rgba[5+j], rgba[6+j]);
        //++j;
        //dark2 = MINT(rgba[++j], rgba[++j], rgba[++j]);
        sum1 += min1;
        sum2 += min2;
        out[i] = min1;
        out[i+1] = min2;
    }
    long * result = (long *)malloc(sizeof(long) * 2);
    result[0] = sum1 + sum2;
    result[1] = MAX(max1, max2);
    //LOG("getDarkThread   sum%ld  max%ld", result[0], result[1]);
    return (void *)(result);
}


unsigned char FastDehazorCV::getDarkChannel(unsigned char * rgba, unsigned char * out, int width, int height,
                                            unsigned char &max)
{
    const int cnt = 5;
    pthread_t pts[cnt];
    int heightUnit = height >> 2;
    TaskParam params[cnt];
    for(int i = 0; i < cnt; ++i)
    {
        params[i].rgba = rgba;
        params[i].out = out;
        params[i].start = width * i * heightUnit;
        if(i == cnt - 1)
        {
            params[i].end = width * height;
        }
        else
        {
            params[i].end = width * (i + 1) * heightUnit;
        }
        int result = pthread_create(&pts[i], NULL, getDarkThread, (void *)(&params[i]));
        if(result != 0)
        {
            LOG("pthread_create fail. getdark");
        }
    }
    long sum = 0;
    max = 0;
    for(int i = 0; i < cnt; ++i)
    {
        void * result;
        if(0 == pthread_join(pts[i], &result))
        {
            sum += ((long *)result)[0];
            if(max < ((long *)result)[1]) max = ((long *)result)[1];
            delete [] ((long *)result);
        }
        else
        {
            LOG("getDarkChannel join fail ");
        }
    }
    return (unsigned char)(sum / width / height);
}

void * getLxThread(void *args)
{
    TaskParam * param = (TaskParam*)args;
    unsigned char a;
    unsigned char b;
    for(int i = param->start; i < param->end; ++i)
    {
        b = param->dark[i];
        a = (unsigned char)( param->p * param->rgba[i]);
        param->out[i] = a < b ? a : b;
    }
    return (void *)0;
}

void getLx(unsigned char * lx, unsigned char * mave, unsigned char * dark, float p, int len)
{
    const int cnt = 5;
    pthread_t pts[cnt];
    int unit = len >> 2;
    TaskParam params[cnt];
    for(int i = 0; i < cnt; ++i)
    {
        params[i].rgba = mave;
        params[i].out = lx;
        params[i].start = unit * i;
        params[i].dark = dark;
        params[i].p = p;
        if(i == cnt - 1)
        {
            params[i].end = len;
        }
        else
        {
            params[i].end = ( i + 1 ) * unit;
        }
        if(pthread_create(&pts[i], NULL, getLxThread, (void *)(&params[i])) != 0)
        {
            LOG("getLx create fail");
        }
    }
    for(int i = 0; i < cnt; ++i)
    {
        if(pthread_join(pts[i], NULL) != 0)
        {
            LOG("getLx join fail");
        }
    }
}

void * getResultThread(void *args)
{
    const float tss = 1.0f / FastDehazorCV::mThreshold;
    float scale;
    TaskParam *param = (TaskParam*)args;
    unsigned char *rgba = param->rgba;
    unsigned char *lx = param->dark;
    unsigned char *table = param->out;

    unsigned char result, r, g, b;
    for(int i = param->start, index = (i << 2), value; i < param->end; ++i, index+=4)
    {
        value = rgba[index] ;
        value = value << 8;
        value +=  lx[i];
        r = table[value ];
        value = rgba[1+index] ;
        value = value << 8;
        value +=  lx[i];
        g = table[value];

        value = rgba[2+index] ;
        value = value << 8;
        value +=  lx[i];
        b = table[value ];
        result = MINT(r, g, b);
        if(result < FastDehazorCV::mThreshold)
        {
            scale = result * tss;
            rgba[index] = (1 - scale) * rgba[index] + scale * r;
            rgba[index+1] = (1 - scale) * rgba[index+1] + scale * g;
            rgba[index+2] = (1 - scale) * rgba[index+2] + scale * b;
        } else{
            rgba[index] = r;
            rgba[index+1] = g;
            rgba[index+2] = b;

        }

    }
    return (void *)0;
}

void getResult(unsigned char * rgba, unsigned char * lx, unsigned char * table, int len)
{
    const int cnt = 5;
    pthread_t pts[cnt];
    int unit = len >> 2;
    TaskParam params[cnt];
    for(int i = 0; i < cnt; ++i)
    {
        params[i].rgba = rgba;
        params[i].dark = lx;
        params[i].out = table;
        params[i].start = unit * i;
        if(i == cnt - 1)
        {
            params[i].end = len;
        }
        else
        {
            params[i].end = (i + 1) * unit;
        }
        if(pthread_create(&pts[i], NULL, getResultThread, (void *)(&params[i])) != 0)
        {
            LOG("getResult create fail");
        }
    }
    for(int i = 0; i < cnt; ++i)
    {
        if(pthread_join(pts[i], NULL) != 0)
        {
            LOG("getResult join fail");
        }
    }
}



int FastDehazorCV::process(unsigned char * rgba, int width, int height, int boxRadius, float p, int t)
{
    if (rgba == NULL || width < 1 || height < 1 || boxRadius < 1 || p < 0.2)
    {
        return INPUT_NULL;
    }
    mThreshold = t;
    mP = p;
    mRadius = boxRadius;
    Mat inputMat(height, width, CV_8UC4, rgba);
    const int LEN = width * height;
    const int WIN_SIZE = 2 * mRadius + 1;



    LOG("dark start");
    //求暗通道， 步骤2
    unsigned char rowMax = 0;
    unsigned char * darkChannel = (unsigned char*)malloc(sizeof(unsigned char) * LEN);
    unsigned char darkmean = getDarkChannel(rgba, darkChannel, width, height, rowMax);
    Mat darkChanMat(height, width, CV_8UC1, darkChannel);
    LOG("dark end");


    //步骤4
    float pmav = mP * (darkmean / 255.0f);  //p * mav;
    if(pmav > MAX_P) pmav = MAX_P;

    //暗通道均值滤波   步骤3
    LOG("mean start");
    cv::Mat mave(height, width, CV_8UC1, darkChannel);
    cv::blur(darkChanMat, mave, cv::Size(WIN_SIZE, WIN_SIZE));
    LOG("mean end");


    //步骤5
    LOG("lx start");
    unsigned char * lx = (unsigned char *)malloc(sizeof(unsigned char) * LEN);
    unsigned char * ptrmave = (unsigned char *)mave.data;
    getLx(lx, ptrmave, darkChannel,pmav, LEN);
    LOG("lx end");


    //求A   步骤6
    LOG("a start");
    unsigned char maveMax = 0;
    for (int i = 0; i < LEN; ++i)
    {
        if(ptrmave[i] > maveMax)
        {
            maveMax = ptrmave[i];
        }
    }
    mAir = (unsigned char)((maveMax + rowMax) / 2);
    LOG("a end");


    LOG("table start");
    InitResultTable();
    LOG("table end");

    //计算输出
    LOG("result start");
    getResult(rgba, lx, mResultTable, LEN);
    LOG("result end");

    free(lx);
    free(darkChannel);
    LOG("return end");

    return 0;
}