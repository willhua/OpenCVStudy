//
// Created by lisan on 2017/4/16.
//
#include "LyhDehazor.h"

#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
//#include "edge_filter.hpp"

using namespace cv;


/***************daryGray********************/


//TODO: 这里应该用查表法优化
void *darkGrayThread(void *args)
{
    ThreadParam *param = (ThreadParam *)args;
    LOG("start%d   end%d   ", param->start, param->end);
    int end = param->end;
    for(int i = param->start, j = i * 4; i < end; ++i, j+=4)
    {
        LOG("d1");
        param->r[i] = param->rgba[j];
        param->g[i] = param->rgba[j + 1];
        param->b[i] = param->rgba[j + 2];
        LOG("d2  %d    end%d", i, param->end);
        UCHAR uu = MINT(param->rgba[j], param->rgba[j + 1], param->rgba[j + 2]);
        param->dark[i] = uu;
        param->gray[i] = (UCHAR)((param->rgba[j] * 1224 + param->rgba[j + 1] * 2404 +param->rgba[j + 2] * 467) >> 12);
        LOG("d3");
    }
}

/**
 * 分别得到rgb三个通道； 得到暗通道； 得到灰度图
 * @param rgba
 * @param r
 * @param g
 * @param b
 * @param dark
 * @param gray
 * @param w
 * @param h
 */
void LyhDehazor::DarkGray(UCHAR * rgba, UCHAR * r, UCHAR * g, UCHAR *b, UCHAR *dark, UCHAR *gray, int w, int h)
{
    const int cnt = 5;
    pthread_t pts[cnt];
    ThreadParam params[cnt];
    int unit = h >> 2;
    for(int i = 0; i < cnt; ++i)
    {
        params[i].start = i * w * unit;
        params[i].rgba = rgba;
        params[i].r = r;
        params[i].g = g;
        params[i].b = b;
        params[i].dark = dark;
        params[i].gray = gray;
        if(i == cnt - 1)
        {
            params[i].end = w * h;
        }
        else
        {
            params[i].end = w * (i + 1) * unit;
        }
        if(0 != pthread_create(&pts[i], NULL, darkGrayThread, (void *)(&params[i])))
        {
            LOG("ERROE  create darkGray %d", i);
        }
    }
    for(int i = 0; i < cnt; ++i)
    {
        if(pthread_join(pts[i], NULL) != 0)
        {
            LOG("ERROE  JOIN darkGray %d", i);
        }
    }
}


/********************************************/