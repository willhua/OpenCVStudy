//
// Created by lisan on 2017/4/11.
//

#include "FastDehazorCV.h"

using namespace cv;

FastDehazorCV::FastDehazorCV(int width, int height)
{
    mP = 2.0f;
    mResultTable = (unsigned char *)malloc(sizeof(unsigned char) * 256 * 256);
    //InitResultTable();
    mUpper = 0.9f;
 /*   mDivN = (int *)malloc(sizeof(int) * width * height);
    mWinSize = 50;
    BoxDivN(mDivN, width, height, mWinSize); */
}

FastDehazorCV::~FastDehazorCV()
{
    if(mResultTable != NULL)
    {
        delete [] mResultTable;
    }
}

void FastDehazorCV::InitResultTable()
{
    float air = 1.0f / mAir;
    int index = 0;
    float result;
    for(int i = 0; i < 256; ++i)
    {
        for(int j = 0; j < 256; ++j)
        {
            result = (i - j) / (1 - j * air);
            mResultTable[index++] = (unsigned char)CLAM(result);
        }
    }
}

int FastDehazorCV::process(unsigned char * rgba, int width, int height, int boxRadius)
{
    if (rgba == NULL || width < 1 || height < 1 || boxRadius < 1)
    {
        return INPUT_NULL;
    }

    Mat inputMat(height, width, CV_8UC4, rgba);
    const int LEN = width * height;
    const int WIN_SIZE = 2 * boxRadius + 1;



    //求暗通道， 步骤2
    unsigned char * darkChannel = (unsigned char *)malloc(sizeof(char) * LEN);
    long long darkSum = 0;
    for(int i = 0, j = 0; j < LEN; i+=4, ++j)
    {
        darkChannel[j] = MINT(rgba[i], rgba[i+1],rgba[i+2]);
        darkSum += darkChannel[j];
    }
    Mat darkChanMat(height, width, CV_8UC1, darkChannel);

    //步骤4
    float pmav = mP * (darkSum / LEN / 255.0f);  //p * mav;
    if(pmav > 0.9f) pmav = 0.9f;

    //暗通道均值滤波   步骤3
    //int * mave = (int *)malloc(sizeof(int) * LEN);
    LOG("mean start");
    //MeanFilter(darkChannel, mave, mWinSize, width, height);
    cv::Mat mave(height, width, CV_8UC1, darkChannel);
    cv::blur(darkChanMat, mave, cv::Size(WIN_SIZE, WIN_SIZE));
    LOG("mean end");


    //步骤5
    unsigned char * lx = (unsigned char *)malloc(sizeof(unsigned char) * LEN);
    unsigned char * ptrmave = (unsigned char *)mave.data;
    for(int i = 0; i < LEN; ++i)
    {
        unsigned char a = (unsigned char)(pmav * ptrmave[i]);
        unsigned char b = darkChannel[i];
        lx[i] = a < b ? a : b;
    }
    //cv::Mat lximg(height, width, CV_8UC1, lx);
    //cv::imwrite("lx.jpg", lximg);


    //求A   步骤6
    unsigned char * prt5;
    unsigned char rgbaMax = 0;
    for(int i = 0; i < height; ++i)
    {
        prt5 = rgba + i * width;
        for (int j = 0; j < width; ++j)
        {
            if(prt5[0] > rgbaMax) rgbaMax = prt5[0];
            if(prt5[1] > rgbaMax) rgbaMax = prt5[1];
            if(prt5[2] > rgbaMax) rgbaMax = prt5[2];
            prt5 += 4;
        }
        if(rgbaMax == 255)
        {
            break;
        }
    }
    int maveMax = 0;
    for (int i = 0; i < LEN; ++i)
    {
        if(ptrmave[i] > maveMax)
        {
            maveMax = ptrmave[i];
        }
    }
    mAir = (unsigned char)((maveMax + rgbaMax) / 2);
    InitResultTable();

    //计算输出
    int index,value, lxi;
    for (int i = 0; i < height; ++i)
    {
        index = i * width * 4 - 1;
        lxi = i*width;
        for (int j = 0; j < width; ++j, ++index, ++lxi)
        {
            value = rgba[++index] ;
            value = value << 8;
            value +=  lx[lxi];
            rgba[index] = mResultTable[value ];

            value = rgba[++index] ;
            value = value << 8;
            value +=  lx[lxi];
            rgba[index] = mResultTable[value];

            value = rgba[++index] ;
            value = value << 8;
            value +=  lx[lxi];
            rgba[index] = mResultTable[value ];
        }
    }
    return 0;
}


void FastDehazorCV::MeanFilter(unsigned char *data, int * outdata, int r, int width, int height) {
    //LOG("MeanFilter  beging");
    float *cum = (float *) malloc(sizeof(float) * width * height);
    if (!cum) {
        //LOG("BoxFilter malloc failed!!!!!!");
        return;
    }
    /**********this  ringht this*************/

    const int len = width * height;
    const int block = 4; //

    //cum y
    for (int i = 0; i < width; ++i) {//the first row
        cum[i] = data[i];
    }
    for (int i = width; i < len; i += width) {
        int end = i + width / block * block;
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
    const int R_WIDTH = r * width;
    const int R1_WIDTH = width * (r + 1);
    for (int i = 0 ; i < (r + 1) * width; i += block)  //不用担心end是不是block的整数倍，就算不是，超过的部分也会在后面重新计算正确
    {
        outdata[i] = cum[R_WIDTH+i];
        outdata[i + 1] = cum[R_WIDTH+i + 1];
        outdata[i + 2] = cum[R_WIDTH+i + 2];
        outdata[i + 3] = cum[R_WIDTH+i + 3];
    }
    for (int i = (r + 1) * width; i < (height - r - 1) * width; i += block) {
        outdata[i] = cum[i + R_WIDTH] - cum[i - R1_WIDTH];
        outdata[i + 1] = cum[i + R_WIDTH + 1] - cum[i - R1_WIDTH + 1];
        outdata[i + 2] = cum[i + R_WIDTH + 2] - cum[i - R1_WIDTH + 2];
        outdata[i + 3] = cum[i + R_WIDTH + 3] - cum[i - R1_WIDTH + 3];
    }
    for (int i = height - r - 1; i < height; ++i) {
        int end = width / block * block;
        int outIndex = i * width;
        int topIndex = outIndex - R1_WIDTH;
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
            outdata[outIndex++] = cum[bottomIndex++] - cum[topIndex++];
        }
    }


    //cum x
    for (int y = 0; y < width * height; y += width) {
        cum[y] = outdata[y];  //处理第一列
    }
    for (int y = 0; y < height / 4 * 4; y += 4) {
        //y01234都是每行的行首
        int y0 = y * width, y1 = (y + 1) * width, y2 = (y + 2) * width, y3 = (y + 3) * width, y4 =
                (y + 4) * width;
        //循环展开，每次处理四行，且每次从第二个元素开始，因为第一列已经处理过了
        for (int i = y0 + 1; i < y1; ++i) {  //处理第一行
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
    for (int y = height / 4 * 4; y < height; ++y) {  //处理循环展开后的剩余行
        for (int i = y * width + 1; i < (y+1)*width; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
    }
    //diff x
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < r + 1; ++x) {
            outdata[y * width + x] = cum[y * width + x + r] / mDivN[y * width + x];
        }
        for (int x = r + 1; x < width - r; ++x) {
            outdata[y * width + x] =
                    (cum[y * width + x + r] - cum[y * width + x - r - 1]) /mDivN[y * width + x];
        }
        for (int x = width - r; x < width; ++x) {
            outdata[y * width + x] = (cum[y * width + width - 1] - cum[y * width + x - r - 1]) /
                                     mDivN[y * width + x];
        }
    }

    delete [] cum;
    cum = NULL;
    //LOG("MeanFilter  end");
}


void FastDehazorCV::BoxDivN(int *out, int width, int height, int r)
{
    int m, n, max;
    max = (2 * r + 1) * (2 * r + 1);   //这就是box整个都在图像范围内的情况，
    for (int y = 0, index = -1; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x >= r && x <= width - r - 1 && y >= r && y <= height - r - 1) {
                out[++index] = max;
            } else {
                m = MINT(r,x, width - 1 - x);
                n = MINT(r,y, height - 1 - y);
                out[++index] = (r + 1 + m) * (r + 1 + n);//对于边缘的
            }

        }
    }

    /*   备用  正确
    int * input = (int *)malloc(sizeof(int)*width * height);
    for (int i = 0; i < width * height; ++i)
    {
        input[i] = 1;
    }
    BoxFilter(input, out,r,width,height, 1);   */
}



template<class T1, class T2>
void FastDehazorCV::BoxFilter(T1 *data, T2 *outdata, int r, int width, int height, T2 t) {
    T2 *cum = (T2 *) malloc(sizeof(T2) * width * height);
    if (!cum) {
        //LOG("BoxFilter malloc failed!!!!!!");
        return;
    }
    /**********this  ringht this*************/

    const int len = width * height;
    const int block = 4; //

    //cum y
    for (int i = 0; i < width; ++i) {//the first row
        cum[i] = data[i];
    }
    for (int i = width; i < len; i += width) {
        int end = i + width / block * block;
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
    const int R_WIDTH = r * width;
    const int R1_WIDTH = width * (r + 1);
    for (int i = 0 ; i < (r + 1) * width; i += block)
    {
        outdata[i] = cum[R_WIDTH+i];
        outdata[i + 1] = cum[R_WIDTH+i + 1];
        outdata[i + 2] = cum[R_WIDTH+i + 2];
        outdata[i + 3] = cum[R_WIDTH+i + 3];
    }
    for (int i = (r + 1) * width; i < (height - r - 1) * width; i += block) {
        outdata[i] = cum[i + R_WIDTH] - cum[i - R1_WIDTH];
        outdata[i + 1] = cum[i + R_WIDTH + 1] - cum[i - R1_WIDTH + 1];
        outdata[i + 2] = cum[i + R_WIDTH + 2] - cum[i - R1_WIDTH + 2];
        outdata[i + 3] = cum[i + R_WIDTH + 3] - cum[i - R1_WIDTH + 3];
    }
    for (int i = height - r - 1; i < height; ++i) {
        int end = width / block * block;
        int outIndex = i * width;
        int topIndex = outIndex - R1_WIDTH;
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
            outdata[outIndex++] = cum[bottomIndex++] - cum[topIndex++];
        }
    }


    //cum x
    for (int y = 0; y < width * height; y += width) {
        cum[y] = outdata[y];
    }
    for (int y = 0; y < height / 4 * 4; y += 4) {

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
    for (int y = height / 4 * 4; y < height; ++y) {
        for (int i = y * width + 1; i < (y+1)*width; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
    }
    //diff x
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < r + 1; ++x) {
            outdata[y * width + x] = cum[y * width + x + r];
        }
        for (int x = r + 1; x < width - r; ++x) {
            outdata[y * width + x] =
                    (cum[y * width + x + r] - cum[y * width + x - r - 1]);
        }
        for (int x = width - r; x < width; ++x) {
            outdata[y * width + x] = (cum[y * width + width - 1] - cum[y * width + x - r - 1]) ;
        }
    }

    delete [] cum;
    cum = NULL;
}