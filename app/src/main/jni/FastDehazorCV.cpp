//
// Created by lisan on 2017/4/11.
//

#include "FastDehazorCV.h"
#include <pthread.h>

using namespace cv;

FastDehazorCV::FastDehazorCV()
{
    mP = 1.5f;
    mSkyThreshold = 15;
    mResultTable = (unsigned char *)malloc(sizeof(unsigned char) * 256 * 256);
}

FastDehazorCV::~FastDehazorCV()
{
    if(mResultTable != NULL)
    {
        delete [] mResultTable;
    }
}

void FastDehazorCV::setP(float p)
{
    if(p < 1)
    {
        mP = p;
    }
}

void FastDehazorCV::InitResultTable()
{
    float air = 1.0f / mAir;
    int index = 0;
    float result;
  //  int threshold = mAir - mSkyThreshold;
    for(int i = 0; i < 256; ++i) //符合暗通道的区域
    {
        for(int j = 0; j < 256; ++j)
        {
            result = (i - j) / (1 - j * air);
            mResultTable[index++] = (unsigned char)CLAM(result);
        }
    }
 /*   for (int i = threshold; i < 256; ++i) {//天空区域，需要增大透射率
        for(int j = 0; j < 256; ++j)
        {
            float scale = ABS(mAir - j) / (float)75;
            int newlx = MIN(mAir, scale * j);
            result = (i - newlx) / (1 - newlx * air);
            LOG("inittable       old %d   new  %d", j, newlx);
            mResultTable[index++] = (unsigned char)CLAM(result);
        }
    }   */
}



typedef struct GetDarkParam
{
    unsigned char * rgba;
    unsigned char * out;
    int start;  //以像素为单位的，即对于out可以直接使用，但是对于rgba则需要乘以4
    int end;
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
    JNIEnv* env = NULL;
    long sum1 = 0;
    long sum2 = 0;
    unsigned char max1;
    unsigned char max2;
    //LOG("gvm %d", JniEnvInit::gVM);
    if(0 == JniEnvInit::gVM->AttachCurrentThread(&env, NULL))
    {
        GetDarkParam * param = (GetDarkParam *)args;
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
        JniEnvInit::gVM->DetachCurrentThread();
    }
    long * result = (long *)malloc(sizeof(long) * 2);
    result[0] = sum1 + sum2;
    result[1] = MAX(max1, max2);
    LOG("getDarkThread   sum%ld  max%ld", result[0], result[1]);
    return (void *)(result);
}


unsigned char FastDehazorCV::getDarkChannel(unsigned char * rgba, unsigned char * out, int width, int height,
                                            unsigned char &max)
{
    const int cnt = 5;
    pthread_t pts[cnt];
    int heightUnit = height >> 2;
    GetDarkParam params[cnt];
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







int FastDehazorCV::process(unsigned char * rgba, int width, int height, int boxRadius)
{
    if (rgba == NULL || width < 1 || height < 1 || boxRadius < 1)
    {
        return INPUT_NULL;
    }

    Mat inputMat(height, width, CV_8UC4, rgba);
    const int LEN = width * height;
    const int WIN_SIZE = 2 * boxRadius + 1;



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
    for(int i = 0; i < LEN; ++i)
    {
        unsigned char a = (unsigned char)(pmav * ptrmave[i]);
        unsigned char b = darkChannel[i];
        lx[i] = a < b ? a : b;
    }
    LOG("lx end");


    //求A   步骤6
    LOG("a start");
 //   unsigned char * prt5;
 //   double rgbaMax = 0;
 //   double rgbaMin = 0;
  //  cv::minMaxLoc(inputMat, &rgbaMin, &rgbaMax);
 /*   for(int i = 0; i < height; ++i)
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
    }  */
    unsigned char maveMax = 0;
//    cv::minMaxLoc(mave, &rgbaMin, &maveMax);
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
    LOG("result end");

    delete [] mResultTable;
    delete [] lx;
    delete [] darkChannel;
    return 0;
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

  /*    // 备用  正确
    int * input = (int *)malloc(sizeof(int)*width * height);
    int * out2 = (int *)malloc(sizeof(int)*width * height);
    for (int i = 0; i < width * height; ++i)
    {
        input[i] = 1;
    }
    BoxFilter(input, out2,r,width,height, 1);
    for(int i = 0; i < width * height; ++i)
    {
        if(out[i] != out2[i])
        {
            LOG("wrong   not equal");
        }
    }
    LOG("    equal");  */
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