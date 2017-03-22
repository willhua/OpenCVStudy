//
// Created by willhua on 2017/3/21.
//
#include "Dehazor.h"

void dehazor(uchar *rgba, int width, int height) {

}


/**
 *
 * @param data
 * @param outdata 
 * @param r  the radius of kernel,then the width and height is 2r+1
 * @param width
 * @param height
 */
void BoxFilter(uchar *data, uchar *outdata, int r, int width, int height) {
    uchar *cum = (uchar *) malloc(sizeof(uchar) * width * height);
    if (!cum) {
        LOG("cumfilter malloc failed!!!!!!");
        return NULL;
    }
    int len = width * height;
    int end;
    const int block = 4; //


    //cum y
    for (int i = 0; i < width; ++i) {//the first row
        cum[i] = data[i];
    }
    for (int i = width; i < len; i += width) {
        end = (i + width) / 4 * 4;
        for (int j = i; j < end; j += block) {
            cum[j] = cum[j - width] + data[j];
            cum[j + 1] = cum[j - width + 1] + data[j + 1];
            cum[j + 2] = cum[j - width + 2] + data[j + 2];
            cum[j + 3] = cum[j - width + 3] + data[j + 3];
        }
        for (int j = end; j < i + width; ++j) {
            cum[j] = cum[j - width] + data[j];
        }
    }
    //diff y
    end = (r + 1) * width;
    for (int i = 0; i < end; i += block)  //不需要担心end是不是block的整数倍，就算不是，超过的部分也会在后面重新计算正确的值
    {
        outdata[i] = cum[i];
        outdata[i + 1] = cum[i + 1];
        outdata[i + 2] = cum[i + 2];
        outdata[i + 3] = cum[i + 3];
    }
    int space = width * (r + 1);
    for (int i = end, end = (height - r - 1) * width; i < end; i += block) {
        outdata[i] = cum[i + space] - cum[i - space];
        outdata[i + 1] = cum[i + space + 1] - cum[i - space + 1];
        outdata[i + 2] = cum[i + space + 2] - cum[i - space + 2];
        outdata[i + 3] = cum[i + space + 3] - cum[i - space + 3];
    }
    for (int i = height - r - 1; i < height; ++i) {
        end = width / block * block;
        int outIndex = i * width;
        int topIndex = outIndex - (r + 1) * width;
        int bottomIndex = (height - 1) * width;
        for (int y = 0; y < end; y += block) {
            outdata[outIndex] = cum[bottomIndex] - cum[topIndex];
            outdata[outIndex + 1] = cum[bottomIndex + 1] - cum[topIndex + 1];
            outdata[outIndex + 2] = cum[bottomIndex + 2] - cum[topIndex + 2];
            outdata[outIndex + 3] = cum[bottomIndex + 3] - cum[topIndex + 3];
            outIndex += block;
            topIndex += block;
            bottomIndex += block;
        }
        for (int y = end; y < width; ++y) {
            outIndex[outIndex++] = cum[bottomIndex++] - cum[topIndex++];
        }
    }


    //cum x
    for (int y = 0; y < width * height; y += width) {
        cum[y] = outdata[y];  //初始化第一列
    }
    end = height / 4 * 4;
    for (int y = 0; y < end; y += 4) {
        int y0 = y * width, y1 = (y + 1) * width, y2 = (y + 2) * width, y3 = (y + 3) * width, y4 =
                (y + 4) * width;
        for (int i = y0 + 1; i < y1; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for (int i = y1 + 1; i < y2; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for (int i = y2 + 1; i < y3; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for (int i = y3 + 1; i < y4; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
    }
    for (int y = end; y < height; ++y) {
        for (int i = y * width + 1, k = 0; k < width; ++k, ++i) {
            cum[i] += outdata[i] + cum[i - 1];
        }
    }
    //diff x
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < r + 1; ++x) {
            outdata[y * height + x] = cum[y * height + x];
        }
        for (int x = r + 1; x < w - r; ++x) {
            outdata[y * height + x] = cum[y * height + x + r] - cum[y * height + x - r - 1];
        }
        for (int x = w - r; x < width; ++x) {
            outdata[y * height + x] = cum[y * height + w - 1] - cum[y * height + x - r - 1];
        }
    }

    delete[] cum;


}
