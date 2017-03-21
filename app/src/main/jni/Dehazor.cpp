//
// Created by willhua on 2017/3/21.
//
#include "Dehazor.h"

void dehazor(uchar *rgba, int width, int height)
{

}


uchar * boxfilter(uchar *data, int width, int height)
{
    uchar *box = (uchar *)malloc(sizeof(uchar) * width * height);
    if(!box)
    {
        LOG("boxfilter malloc failed!!!!!!");
        return NULL;
    }
    int len = width * height;

    const int block = 4; //


    //cum y
    memcpy(box, data, width);  //the first row
    for(int i = width; i < len; i += width)
    {
        int upper = (i + width) / 4 * 4;
        for(int j = i; j < upper; j += block)
        {
            box[j] = box[j - width] + data[j];
            box[j + 1] = box[j - width + 1] + data[j + 1];
            box[j + 2] = box[j - width + 2] + data[j + 2];
            box[j + 3] = box[j - width + 3] + data[j + 3];
        }
        for(int j = upper; j < i + width; ++j)
        {
            box[j] = box[j - width] + data[j];
        }
    }


    //cum x
}
