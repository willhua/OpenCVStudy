#include "LyhDehazor.h"


LyhDehazor::LyhDehazor(int width, int height,  int r)
{
    mDivN = (int *)malloc(sizeof(int) * width * height);
    BoxDivN(mDivN, width, height, r);
}


LyhDehazor::~LyhDehazor(void)
{
    delete [] mDivN;
}

/************************************************************************/
/* 使用尺寸为boxWidth*boxHeight的box对data进行最小值滤波。
    width*height为data的二维尺寸。输出放在out */
/************************************************************************/
void LyhDehazor::MinFilter(UCHAR *data, int width, int height, int boxWidth, int boxHeight, UCHAR *out)
{
    UCHAR replaceMin = 255, reserveMin = 255; //保留部分和替换部分的最小值
    UCHAR *minForRow = (UCHAR*)malloc(sizeof(UCHAR)*height); //保存当前处理的列范围内，每行的的最小值
    if(minForRow == NULL)
    {
        return; //ERROR
    }
    for(int i = 0; i < height; ++i)
    {
        minForRow[i] = 0;
    }
    for(int x = 0; x < width; ++x)
    {
        int left = MAX(0, x - boxWidth), right = MIN(x + boxWidth , width);
        for(int y = 0; y < height; ++y)
        {
            //若是第一次计算minForRow，那么minforrow中的值应该为0，即uchar可能的最小值，如果if成立，那么该行的最小值即为最后一个，为0
            //若minforrow已经经历过一次计算，那么minforrow中含有的是[left - 1, right -2)中的最小值，现在我们要求的是[left,right - 1),
            //那么，如果if成立，data则是整个[left - 1, right -1)中的最小值，自然也是我们要求的最小值
            //如果已经到了末端，那么也没关系，if一直会为false
            if(data[right - 1] <= minForRow[y])
            {
                minForRow[y] = data[right - 1];
                continue;
            }
            minForRow[y] = MinLine(data, width, y, left, right);
        }
        int pre = 0;
        for(int y = 0; y < height; ++y)
        {
            int left = MAX(0, y - boxHeight), right = MIN(y + boxHeight, height);
            if(minForRow[right - 1] <= pre)
            {
                pre = out[y * width + x] = minForRow[right - 1];
            }
            pre = out[y * width + x] = MinLine(minForRow, height, 0, MAX(0, y - boxHeight), right);
        }

    }
}


/************************************************************************/
/* 对应公式12，获得透射率的预估.   data为min[I(y)/A]  */
/************************************************************************/
void LyhDehazor::GetTrans(float *data, int width, int height, int boxsize, float *out, float w) {
    float replaceMin = 255, reserveMin = 255;//保留部分和替换部分的最小值
    float *minForRow = (float *) malloc(sizeof(float) * height); //保存当前处理的列范围内，每行的的最小值
    if (minForRow == NULL) {
        return; //ERROR
    }
    for (int i = 0; i < height; ++i) {
        minForRow[i] = 0;
    }
    for (int x = 0; x < width; ++x) {
        int left = MAX(0, x - boxsize), right = MIN(x + boxsize, width);
        for (int y = 0; y < height; ++y) {
            //若是第一次计算minForRow，那么minforrow中的值应该为0，即uchar可能的最小值，如果if成立，那么该行的最小值即为最后一个，为0
            //若minforrow已经经历过一次计算，那么minforrow中含有的是[left - 1, right -2)中的最小值，现在我们要求的是[left,right - 1),
            //那么，如果if成立，data则是整个[left - 1, right -1)中的最小值，自然也是我们要求的最小值
            //如果已经到了末端，那么也没关系，if一直会为false
            if (data[right - 1] <= minForRow[y]) {
                minForRow[y] = data[right - 1];
                continue;
            }
            minForRow[y] = MinLine(data, width, y, left, right);
        }
        float pre = 0;
        for (int y = 0; y < height; ++y) {
            int left = MAX(0, y - boxsize), right = MIN(y + boxsize, height);
            if (minForRow[right - 1] <= pre) {
                pre = minForRow[right - 1];
                out[y * width + x] = 1 - pre * w;
            }
            pre = minForRow[right - 1];
            out[y * width + x] = 1 - pre * w;
        }

    }
}

/************************************************************************/
/* 找到data中第line行的[left, right)范围内的最小值                      */
/************************************************************************/
UCHAR LyhDehazor::MinLine(UCHAR *data, int width, int line, int left, int right)
{
    int l = width * line + left, r = width * line + right;
    UCHAR min = 255;
    for(int i = l; i < r; ++i)
    {
        if(data[i] < min)
        {
            min = data[i];
    
        }
    }
    return min;
}

/************************************************************************/
/* 找到data中第line行的[left, right)范围内的最小值                                                                      */
/************************************************************************/
float LyhDehazor::MinLine(float *data, int width, int line, int left, int right)
{
    int l = width * line + left, r = width * line + right;
    float min = 255;
    for(int i = l; i < r; ++i)
    {
        if(data[i] < min)
        {
            min = data[i];

        }
    }
    return min;
}

void LyhDehazor::Dehazor(UCHAR *imageDataRGBA, int width, int height)
{
    const int LEN = width * height;
    UCHAR *oriR = (UCHAR*)malloc(sizeof(UCHAR) * LEN);  //原图像的r通道
    UCHAR *oriG = (UCHAR*)malloc(sizeof(UCHAR) * LEN);   //g
    UCHAR *oriB = (UCHAR*)malloc(sizeof(UCHAR) * LEN);
    UCHAR *oriDark = (UCHAR*)malloc(sizeof(UCHAR) * LEN);  //原图像的暗通道
    UCHAR *Air = (UCHAR*)malloc(sizeof(UCHAR) * 3);

    UCHAR low = 255, heigh = 0;
    for(int i = 0, j = 0; i < 4* LEN; ++j,i+=4) //因为是rgba
    {
        oriR[j] = imageDataRGBA[i];
        oriG[j]=imageDataRGBA[i+1];
        oriB[j]=imageDataRGBA[i+2];
        oriDark[j]=MINT(imageDataRGBA[i],imageDataRGBA[i+1],imageDataRGBA[i+2]);//在这一步可以考虑通过计算暗通道的平均值，判断是否属于有雾图片
        if(oriDark[j] < low)
        {
            low = oriDark[j];
        }
        if(oriDark[j] > heigh)
        {
            heigh = oriDark[j];
        }
    }


    UCHAR *oriMinDark = (UCHAR*)malloc(sizeof(UCHAR) * LEN);  //原图像的暗通道
    MinFilter(oriDark, width, height, RADIUS, RADIUS, oriMinDark);
    AirlightEsimation(imageDataRGBA, oriMinDark, Air, LEN, low, heigh);
    float *oriImageDivAirDark = (float*)malloc(sizeof(float) * LEN);
    float *trans = (float*)malloc(sizeof(float) * LEN);
    ImageDivAir(imageDataRGBA, LEN, oriImageDivAirDark, Air);
    GetTrans(oriImageDivAirDark, width, height, RADIUS, trans, 0.95f);
    
    GuidedFilter(oriR, oriG, oriB, trans, width, height, RADIUS, 0.1f);

    //以上，计算得到透射率
    /////////////////////////////////////////////////////////
    const float MIN_T = 0.1f;
    for (int i = 0, j = 0; i < 4 * LEN; i += 4, ++j)
    {
        float v = (imageDataRGBA[i] - Air[0]) / MAX(0.1f, trans[j]) + Air[0];
        imageDataRGBA[i] = CLAM(v);
        v = (imageDataRGBA[i+1] - Air[1]) / MAX(0.1f, trans[j]) + Air[1];
        imageDataRGBA[i+1] = CLAM(v);
        v = (imageDataRGBA[i+2] - Air[2]) / MAX(0.1f, trans[j]) + Air[2];
        imageDataRGBA[i+2] = CLAM(v);
    }

}

//out:输出A 的rgb值
void LyhDehazor::AirlightEsimation(UCHAR *oriImage, UCHAR *oriDark, UCHAR *out, int len, int low, int heigh)
{
    //需要增加一个最大值，计算得到的值不能超过最大值 http://www.cnblogs.com/Imageshop/p/3281703.html

    ////////////////////////
    /*
    * 大气强度计算估计方式：
    *假设暗通道的最小值为L，最大值为H，想要计算的点数为S（即论文中的0.1%，即0.001*len）
    *假设L-H的为线性分布，则令M=H-(H-L)*S/len，即线性分布时，取S个点时的理论阈值（即大于M的点的个数刚好为S个）
    *现在是已经取的0.1%，是很小的区域了，而且对于有雾的图片来说，我们的目标点是集中于块状之中的，
    *所以可以认为在M-L范围内，值是均匀分布的，突变的数量较少
    *于是假设实际中，大于M的值的个数为LM，如果LM>S，则意味着在M-L区域内的值密度低，且阈值是肯定要大于M,
    *如果LM<S，则意味着在M-L区域内值密度高，且阈值肯定要小于M
    *根据以上推论设计的阈值计算公式为：T=M+(LM-S)/LM*(H-M);
    */

    /////////////////
    const float SCALE = 0.001f;
    const int m = (int)(heigh - (heigh - low) * SCALE);
    int num = 0;
    for (int i = 0; i < len; ++i)
    {
        if (oriDark[i] > m)
        {
            ++num;
        }
    }
    const int threshold = (int)(m + (num - SCALE * len) / num * (heigh - m));
    num = 0;
    int r = 0, g = 0, b = 0;
    for (int i = 0; i < len; ++i)
    {
        if (oriDark[i] > threshold)
        {
            r += oriImage[i];
            g += oriImage[i + 1];
            b += oriImage[i + 2];
            ++num;
        }
    }
    out[0] = r / num;
    out[1] = g / num;
    out[2] = b / num;

}

//对应公式12中的I(y)/A，即查找原图除以大气光之后的暗通道  . len = width*height
void LyhDehazor::ImageDivAir(UCHAR *oriImage, int len, float *out, UCHAR *air)
{
    float air_r = air[0], air_g = air[1], air_b = air[2];
    for(int i = 0, j = 0; i < len * 3; j++,i+=3)
    {
        out[j] = MINT(oriImage[i] / air_r, oriImage[i+1]/air_g, oriImage[i+2]/air_b);
    }
}


/**
 * 以r为宽高，把范围内所有值合并到outdata中，然后再除以r*r,即使用box进行均值滤波
 * @param data
 * @param outdata
 * @param r  the radius of kernel,then the width and height is 2r+1
 * @param width
 * @param height
 * @param cumtype   对于T1为UCHAR则T2为int, float则为float.   这个参数的值没有用到
 */
template<class T1, class T2>void LyhDehazor::MeanFilter(T1 *data, T1 *outdata, int r, int width, int height , T2 cumtype) 
{
    T2 *cum = (T2 *) malloc(sizeof(T2) * width * height);
    if (!cum) {
        //LOG("BoxFilter malloc failed!!!!!!");
        return;
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
    int i = 0;
    end = (r + 1) * width;
    for (i = 0; i < end; i += block)   //不需要担心end是不是block的整数倍，就算不是，超过的部分也会在后面重新计算正确的值
    {
        outdata[i] = cum[i];
        outdata[i + 1] = cum[i + 1];
        outdata[i + 2] = cum[i + 2];
        outdata[i + 3] = cum[i + 3];
    }
    int space = width * (r + 1);
    for (i = end, end = (height - r - 1) * width; i < end; i += block) {
        outdata[i] = cum[i + space] - cum[i - space];
        outdata[i + 1] = cum[i + space + 1] - cum[i - space + 1];
        outdata[i + 2] = cum[i + space + 2] - cum[i - space + 2];
        outdata[i + 3] = cum[i + space + 3] - cum[i - space + 3];
    }
    for (i = height - r - 1; i < height; ++i) {
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
            outdata[outIndex++] = cum[bottomIndex++] - cum[topIndex++];
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
        for ( i = y0 + 1; i < y1; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for ( i = y1 + 1; i < y2; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for ( i = y2 + 1; i < y3; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for ( i = y3 + 1; i < y4; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
    }
    for (int y = end; y < height; ++y) {
        for (int i = y * width + 1, k = 0; k < width; ++k, ++i) {
            cum[i] += outdata[i] + cum[i - 1];
        }
    }
    //diff x
    int N = r * r;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < r + 1; ++x) {
            outdata[y * height + x] = cum[y * height + x] / mDivN[y * height + x];
        }
        for (int x = r + 1; x < width - r; ++x) {
            outdata[y * height + x] = (cum[y * height + x + r] - cum[y * height + x - r - 1]) / mDivN[y * height + x];
        }
        for (int x = width - r; x < width; ++x) {
            outdata[y * height + x] = (cum[y * height + width - 1] - cum[y * height + x - r - 1]) / mDivN[y * height + x];
        }
    }

    delete[] cum;
}


template<class T1, class T2>void LyhDehazor::BoxFilter(T1 *data, T1 *outdata, int r, int width, int height , T2 cumtype) 
{
    T2 *cum = (T2 *) malloc(sizeof(T2) * width * height);
    if (!cum) {
        //LOG("BoxFilter malloc failed!!!!!!");
        return;
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
    int i = 0;
    end = (r + 1) * width;
    for (i = 0; i < end; i += block)   //不需要担心end是不是block的整数倍，就算不是，超过的部分也会在后面重新计算正确的值
    {
        outdata[i] = cum[i];
        outdata[i + 1] = cum[i + 1];
        outdata[i + 2] = cum[i + 2];
        outdata[i + 3] = cum[i + 3];
    }
    int space = width * (r + 1);
    for (i = end, end = (height - r - 1) * width; i < end; i += block) {
        outdata[i] = cum[i + space] - cum[i - space];
        outdata[i + 1] = cum[i + space + 1] - cum[i - space + 1];
        outdata[i + 2] = cum[i + space + 2] - cum[i - space + 2];
        outdata[i + 3] = cum[i + space + 3] - cum[i - space + 3];
    }
    for (i = height - r - 1; i < height; ++i) {
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
            outdata[outIndex++] = cum[bottomIndex++] - cum[topIndex++];
        }
    }


    //cum x
    for (int y = 0; y < width * height; y += width) {
        cum[y] = outdata[y];  //��ʼ����һ��
    }
    end = height / 4 * 4;
    for (int y = 0; y < end; y += 4) {
        int y0 = y * width, y1 = (y + 1) * width, y2 = (y + 2) * width, y3 = (y + 3) * width, y4 =
            (y + 4) * width;
        for ( i = y0 + 1; i < y1; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for ( i = y1 + 1; i < y2; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for ( i = y2 + 1; i < y3; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for ( i = y3 + 1; i < y4; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
    }
    for (int y = end; y < height; ++y) {
        for (int i = y * width + 1, k = 0; k < width; ++k, ++i) {
            cum[i] += outdata[i] + cum[i - 1];
        }
    }
    //diff x
    int N = r * r;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < r + 1; ++x) {
            outdata[y * height + x] = cum[y * height + x];
        }
        for (int x = r + 1; x < width - r; ++x) {
            outdata[y * height + x] = cum[y * height + x + r] - cum[y * height + x - r - 1];
        }
        for (int x = width - r; x < width; ++x) {
            outdata[y * height + x] = cum[y * height + width - 1] - cum[y * height + x - r - 1];
        }
    }

    delete[] cum;
}

//此函数的效果等于：A = data1 .* data2； 然后再MeanFilter(A)
template<class T1, class T2, class T3> void LyhDehazor::MulMeanFilter(T1 *data1, T2 *data2, T3 *outdata, int r, int width, int height)
{
    T3 *cum = (T3 *) malloc(sizeof(T3) * width * height);
    if (!cum) {
        //LOG("BoxFilter malloc failed!!!!!!");
        return;
    }
    int len = width * height;
    int end;
    const int block = 4; //


    //cum y
    for (int i = 0; i < width; ++i) {//the first row
        cum[i] = data1[i] * data2[i];
    }
    for (int i = width; i < len; i += width) {
        end = (i + width) / 4 * 4;
        for (int j = i; j < end; j += block) {
            cum[j] = cum[j - width] + data1[j] * data2[j];
            cum[j + 1] = cum[j - width + 1] + data1[j + 1] * data2[j + 1];
            cum[j + 2] = cum[j - width + 2] + data1[j + 2] * data2[j + 2];
            cum[j + 3] = cum[j - width + 3] + data1[j + 3] * data2[j + 3];
        }
        for (int j = end; j < i + width; ++j) {
            cum[j] = cum[j - width] + data1[j] * data2[j];
        }
    }
    //diff y
    int i = 0;
    end = (r + 1) * width;
    for (i = 0; i < end; i += block)  //不需要担心end是不是block的整数倍，就算不是，超过的部分也会在后面重新计算正确的值
    {
        outdata[i] = cum[i];
        outdata[i + 1] = cum[i + 1];
        outdata[i + 2] = cum[i + 2];
        outdata[i + 3] = cum[i + 3];
    }
    int space = width * (r + 1);
    for (i = end, end = (height - r - 1) * width; i < end; i += block) {
        outdata[i] = cum[i + space] - cum[i - space];
        outdata[i + 1] = cum[i + space + 1] - cum[i - space + 1];
        outdata[i + 2] = cum[i + space + 2] - cum[i - space + 2];
        outdata[i + 3] = cum[i + space + 3] - cum[i - space + 3];
    }
    for (i = height - r - 1; i < height; ++i) {
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
            outdata[outIndex++] = cum[bottomIndex++] - cum[topIndex++];
        }
    }


    //cum x
    for (int y = 0; y < width * height; y += width) {
        cum[y] = outdata[y];  //��ʼ����һ��
    }
    end = height / 4 * 4;
    for (int y = 0; y < end; y += 4) {
        int y0 = y * width, y1 = (y + 1) * width, y2 = (y + 2) * width, y3 = (y + 3) * width, y4 =
            (y + 4) * width;
        for ( i = y0 + 1; i < y1; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for ( i = y1 + 1; i < y2; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for ( i = y2 + 1; i < y3; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
        for ( i = y3 + 1; i < y4; ++i) {
            cum[i] = outdata[i] + cum[i - 1];
        }
    }
    for (int y = end; y < height; ++y) {
        for (int i = y * width + 1, k = 0; k < width; ++k, ++i) {
            cum[i] += outdata[i] + cum[i - 1];
        }
    }
    //diff x
    int N = r * r;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < r + 1; ++x) {
            outdata[y * height + x] = cum[y * height + x] / N;
        }
        for (int x = r + 1; x < width - r; ++x) {
            outdata[y * height + x] = (cum[y * height + x + r] - cum[y * height + x - r - 1]) / N;
        }
        for (int x = width - r; x < width; ++x) {
            outdata[y * height + x] = (cum[y * height + width - 1] - cum[y * height + x - r - 1]) / N;
        }
    }

    delete[] cum;

}

//此函数效果等于：out = input1 .* input2
template<class T1, class T2, class T3> void LyhDehazor::ArrayMul(T1 *input1, T2 *input2, int len, T3 *out)
{
    for(int i = 0; i < len; ++i)
    {
        out[i] = input1[i] * input2[i];
    }
}
//此函数效果等于：exy = exy - ex .* ey
template<class T1, class T2, class T3> void LyhDehazor::Variance(T1 *exy, T2 *ex, T3 ey, int len)
{
    for(int i = 0; i < len; ++i)
    {
        exy[i] = ex[i] - ey[i];
    }
}

//ori_r分别为原图的rgb通道， data为初步的透射率图， 图片尺寸width*height  滤波半径size eps
void LyhDehazor::GuidedFilter(UCHAR *ori_r, UCHAR *ori_g,UCHAR *ori_b, float *data, int width, int height,int size,float e)
{
    int LEN = width * height;
    UCHAR *mr = (UCHAR*)malloc(sizeof(UCHAR)*LEN);
    UCHAR *mg = (UCHAR*)malloc(sizeof(UCHAR)*LEN);
    UCHAR *mb = (UCHAR*)malloc(sizeof(UCHAR)*LEN);
    MeanFilter(ori_r, mr,size, width, height,1);
    MeanFilter(ori_g, mg,size, width, height,1);
    MeanFilter(ori_b, mb,size, width, height,1);

    float *md = (float*)malloc(sizeof(float)*LEN);
    MeanFilter(data, md,size, width, height,1.0f);

    float *mdr = (float*)malloc(sizeof(float)*LEN);
    MulMeanFilter(data, ori_r, mdr, size, width, height);
    for(int i = 0; i < LEN; ++i)
    {
        mdr[i] -= md[i] * mr[i];
    }
    

    float *mdg = (float*)malloc(sizeof(float)*LEN);
    MulMeanFilter(data, ori_g, mdg, size, width, height);
    for(int i = 0; i < LEN; ++i)
    {
        mdg[i] -= md[i] * mg[i];
    }

    float *mdb = (float*)malloc(sizeof(float)*LEN);
    MulMeanFilter(data, ori_b, mdb, size, width, height);
    for(int i = 0; i < LEN; ++i)
    {
        mdb[i] -= md[i] * mb[i];
    }

    int *mrr = (int*)malloc(sizeof(int)*LEN);
    MulMeanFilter(ori_r, ori_r, mrr, size, width, height);
    for(int i = 0; i < LEN; ++i)
    {
        mrr[i] -= (mr[i] * mr[i]);
    }

    int *mrg = (int*)malloc(sizeof(int)*LEN);
    MulMeanFilter(ori_r, ori_g, mrg, size, width, height);
    for(int i = 0; i < LEN; ++i)
    {
        mrg[i] -= (mr[i] * mg[i]);
    }

    int *mrb = (int*)malloc(sizeof(int)*LEN);
    MulMeanFilter(ori_r, ori_b, mrb, size, width, height);
    for(int i = 0; i < LEN; ++i)
    {
        mrb[i] -= (mr[i] * mb[i]);
    }

    int *mbb = (int*)malloc(sizeof(int)*LEN);
    MulMeanFilter(ori_b, ori_b, mbb, size, width, height);
    for(int i = 0; i < LEN; ++i)
    {
        mbb[i] -= (mb[i] * mb[i]);
    }

    int *mbg = (int*)malloc(sizeof(int)*LEN);
    MulMeanFilter(ori_b, ori_g, mbg, size, width, height);
    for(int i = 0; i < LEN; ++i)
    {
        mbg[i] -= (mb[i] * mg[i]);
    }

    int *mgg = (int*)malloc(sizeof(int)*LEN);
    MulMeanFilter(ori_g, ori_g, mgg, size, width, height);
    for(int i = 0; i < LEN; ++i)
    {
        mgg[i] -= (mg[i] * mg[i]);
    }


    float eps = 0.5f;//�ݶ�
    float *ar = (float*)malloc(sizeof(float) * LEN);
    float *ag = (float*)malloc(sizeof(float) * LEN);
    float *ab = (float*)malloc(sizeof(float) * LEN);
    float *b = (float*)malloc(sizeof(float) * LEN);
    for(int i = 0; i < LEN; ++i)
    {
        float v1 = (mgg[i] + eps)*(mbb[i] + eps) - mbg[i] * mbg[i];
        float v2 = mbg[i] * mrb[i] - mrg[i] * (mbb[i] + eps);
        float v3 = (mrr[i] + eps) * (mbb[i] + eps) - mrb[i] * mrb[i];
        float v4 = mrg[i] * mbg[i] - (mgg[i] + eps) * mrb[i];
        float v5 = mrg[i] * mrb[i] - (mrr[i] + eps) * mbg[i];
        float v6 = (mrr[i] + eps)*(mgg[i] + eps) - mrg[i] * mrg[i];

        ar[i] = mdr[i] * v1 + mdg[i] * v2 + mdb[i] * v4;
        ag[i] = mdr[i] * v2 + mdg[i] * v3 + mdb[i] * v5;
        ab[i] = mdr[i] * v4 + mdg[i] * v5 + mdb[i] * v6;
    }
    for(int i = 0; i < LEN; ++i)
    {
        b[i] = md[i] - ar[i] * mr[i] - ag[i] * mg[i] - ab[i] * mb[i];
    }

    delete [] mr; delete [] mg; delete [] mb; delete [] mrr; delete [] mrg; delete [] mrb;delete [] mbg; delete [] mgg; delete [] mbb;
    delete [] md; delete [] mdr; delete [] mdg; delete [] mdb;

    float *boxar = (float*)malloc(sizeof(float) * LEN);
    BoxFilter(ar, boxar, size, width, height, 1.0f);
    float *boxag = (float*)malloc(sizeof(float) * LEN);
    BoxFilter(ag, boxag, size, width, height, 1.0f);
    float *boxab = (float*)malloc(sizeof(float) * LEN);
    BoxFilter(ab, boxab, size, width, height, 1.0f);
    float *boxb = (float*)malloc(sizeof(float) * LEN);
    BoxFilter(b, boxb, size, width, height, 1.0f);

    for(int i = 0; i < LEN; ++i)
    {
        data[i] = (boxar[i] * ori_r[i] + boxag[i] * ori_g[i] + boxab[i] * ori_b[i] + boxb[i]) / mDivN[i];
    }
}



/*
void LyhDehazor::GuidedFilter(UCHAR *guidedImage, float *data, int width, int height, int size, int e)
{
    UCHAR *mg = (UCHAR*)malloc(sizeof(UCHAR) * width * height);
    float *md = (float*)malloc(sizeof(float) * width * height);
    int *mgg = (int*)malloc(sizeof(int) * width * height);
    float *mgd = (float*)malloc(sizeof(float) * width * height);

    MeanFilter(guidedImage, mg, size, width, height, 1);
    MeanFilter(data, md, size, width, height, 0.1f);
    MulMeanFilter(guidedImage, guidedImage, mgg, size, width, height);
    MulMeanFilter(guidedImage, data, mgd, size, width, height);

    float *a = (float*)malloc(sizeof(float) * width * height);
    for(int i = 0; i < width * height; ++i)
    {
        a[i] = (mgd[i] - mg[i] * md[i]) / (mgg[i] - mg[i] * mg[i] + e);
    }

    float *b = (float*)malloc(sizeof(float) * width * height);
    for(int i = 0; i < width * height; ++i)
    {
        b[i] = md[i] - a[i] * mg[i];
    }
    delete [] mg;
    delete [] md;
    delete [] mgg;
    float *mb = (float*)malloc(sizeof(float) * width * height);
    MeanFilter(a, mgd, size, width, height, 1.0f);  //a��mean����mgd��
    MeanFilter(b, mb, size, width, height, 1.0f);
    for(int i = 0; i < width * height; ++i)
    {
        data[i] = mgd[i] * guidedImage[i] + mb[i]; //����������data��
    }
    delete [] mgd;
    delete [] mb;
 }


*/

//用来创建均值时的除数。因为图像边缘的不需要除以width*height
void LyhDehazor::BoxDivN(int *out, int width, int height, int r)
{
    int m, n, max;
    max = (2*r+1)*(2*r+1);   //这就是box整个都在图像范围内的情况，
    for(int x = 0, index = 0; x < width; ++x)
    {
        for(int y = 0; y < height; ++y)
        {
            if(x >= r && x <= width -r -1 && y >= r && y <= height - r - 1)
            {
                out[index++] = max;

            }else
            {
                m = MIN(x, width -1- x);
                n = MIN(y, height-1 - y);
                out[index++] = (r+1+m)*(r+1+n);//对于边缘的
            }

        }
    }
}