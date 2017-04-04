#include "LyhDehazor.h"
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "lyhopencvn", __VA_ARGS__)


LyhDehazor::LyhDehazor(int width, int height, int r) {
    mDivN = (int *) malloc(sizeof(int) * width * height);
    BoxDivN(mDivN, width, height, r);
    mRadius = r;
	for(int i = 0; i < 450*600;i+=100)
	{
		int a = mDivN[i];
	}
}


LyhDehazor::~LyhDehazor(void) {
    delete[] mDivN;
}

/************************************************************************/
/* 使用尺寸为boxWidth*boxHeight的box对data进行最小值滤波。
    width*height为data的二维尺寸。输出放在out */
/************************************************************************/
void
LyhDehazor::MinFilter(unsigned char *data, int width, int height, int boxWidth, int boxHeight, unsigned char *out) {
    //LOG("MinFilter   begin");

    unsigned char replaceMin = 255, reserveMin = 255; //保留部分和替换部分的最小值
    unsigned char *minForRow = (unsigned char *) malloc(sizeof(unsigned char) * height); //保存当前处理的列范围内，每行的的最小值
    if (minForRow == NULL) {
        return; //ERROR
    }
    for (int i = 0; i < height; ++i) {
        minForRow[i] = 0;
    }
    for (int x = 0; x < width; ++x) {
        int left = MAX(0, x - boxWidth), right = MIN(x + boxWidth, width);
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
        int pre = 0;
        for (int y = 0; y < height; ++y) {
            int left = MAX(0, y - boxHeight), right = MIN(y + boxHeight, height);
            if (minForRow[right - 1] <= pre) {
                pre = out[y * width + x] = minForRow[right - 1];
            }
            pre = out[y * width + x] = MinLine(minForRow, height, 0, MAX(0, y - boxHeight), right);
        }

    }
    LOG("MinFilter   end");
}


/************************************************************************/
/* 对应公式12，获得透射率的预估.   data为min[I(y)/A]  */
/************************************************************************/
void LyhDehazor::GetTrans(float *data, int width, int height, int boxsize, float *out, float w) {
	float * minForRow222 = (float *) malloc(sizeof(float) * height*width);
	for(int i =0;i< width*height;++i)
	{
		minForRow222[i] = data[i] * 255;
	}
	writeImgF(width, height, minForRow222, "aaatuosheshuru_my.jpg");
	for(int y = 0; y < height; ++y)
	{
		for(int x = 0; x < width; ++x)
		{
			float min = 255;
			int left = MAX(x - boxsize, 0), top = MAX(y - boxsize, 0), right = MIN(x + boxsize, width - 1), bottom = MIN(y + boxsize, height - 1);
			for(int i = left; i <= right; ++i)
			{
				for(int j = top; j < bottom; ++j)
				{
					if(data[j*width + i] < min)
					{
						min = data[j*width + i];
					}
				}
			}
			out[y*width + x] = (1 - w*min)*255;
		}
	}

	writeImgF(width, height, out, "aaa_trans.jpg");

	/***********************************************************************************/ 
	float * minForRow22 = (float *) malloc(sizeof(float) * height*width);
	for(int i =0;i< width*height;++i)
	{
		minForRow22[i] = data[i] * 255;
	}
	writeImgF(width, height, minForRow22, "aabtuosheshuru_my.jpg");


    float replaceMin = 255, reserveMin = 255;//保留部分和替换部分的最小值
    float * minForRow = (float *) malloc(sizeof(float) * height); //保存当前处理的列范围内，每行的的最小值
    if (minForRow == NULL) {
        return; //ERROR
    }
    for (int i = 0; i < height; ++i) {
        minForRow[i] = 0;
    }
    for (int x = 0; x < width; ++x) {
        int left = MAX(0, x - boxsize), right = MIN(x + boxsize, width);
        for (int y = 0; y < height; ++y) {  //这个for循环计算得到了left-right的宽度范围内的，所有行的最小值
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
                out[y * width + x] = (1 - pre * w)*255;
				//out[y * width + x] = 1 - pre * w;
                continue;
            }
            pre = MinLine(minForRow, height, 0, left, right);
            out[y * width + x] = (1 - pre * w)*255;
			//out[y * width + x] = 1 - pre * w;
        }

    }
	delete [] minForRow;   
	writeImgF(width, height, out, "aab_trans.jpg");
}

/************************************************************************/
/* 找到data中第line行的[left, right)范围内的最小值                      */
/************************************************************************/
unsigned char LyhDehazor::MinLine(unsigned char *data, int width, int line, int left, int right) {
    int l = width * line + left, r = width * line + right;
    unsigned char min = 255;
    for (int i = l; i < r; ++i) {
        if (data[i] < min) {
            min = data[i];

        }
    }
    return min;
}

/************************************************************************/
/* 找到data中第line行的[left, right)范围内的最小值                                                                      */
/************************************************************************/
float LyhDehazor::MinLine(float *data, int width, int line, int left, int right) {
    int l = width * line + left, r = width * line + right;
    float min = 255;
    for (int i = l; i < r; ++i) {
        if (data[i] < min) {
            min = data[i];

        }
    }
    return min;
}

void LyhDehazor::Dehazor(unsigned char *imageDataRGBA, int width, int height) {
    const int LEN = width * height;
    unsigned char *oriR = (unsigned char *) malloc(sizeof(unsigned char) * LEN);  //原图像的r通道
    unsigned char *oriG = (unsigned char *) malloc(sizeof(unsigned char) * LEN);   //g
    unsigned char *oriB = (unsigned char *) malloc(sizeof(unsigned char) * LEN);
    unsigned char *oriDark = (unsigned char *) malloc(sizeof(unsigned char) * LEN);  //原图像的暗通道
    unsigned char *Air = (unsigned char *) malloc(sizeof(unsigned char) * 3);

    unsigned char low = 255, heigh = 0;
    for (int i = 0, j = 0; i < 4 * LEN; ++j, i += 4) //因为是rgba
    {
        oriR[j] = imageDataRGBA[i];
        oriG[j] = imageDataRGBA[i + 1];
        oriB[j] = imageDataRGBA[i + 2];
        oriDark[j] = MINT(imageDataRGBA[i], imageDataRGBA[i + 1],
                          imageDataRGBA[i + 2]);//在这一步可以考虑通过计算暗通道的平均值，判断是否属于有雾图片
        if (oriDark[j] < low) {
            low = oriDark[j];
        }
        if (oriDark[j] > heigh) {
            heigh = oriDark[j];
        }
    }
	writeImg(width, height, oriDark, "dark-no-filter.jpg");

    unsigned char *oriMinDark = (unsigned char *) malloc(sizeof(unsigned char) * LEN);  //原图像的暗通道
    MinFilter(oriDark, width, height, mRadius, mRadius, oriMinDark);
	writeImg(width, height, oriMinDark, "dark-filter.jpg");
    AirlightEsimation(imageDataRGBA, oriMinDark, Air, LEN, low, heigh);
    float *oriImageDivAirDark = (float *) malloc(sizeof(float) * LEN);
    ImageDivAir(imageDataRGBA, LEN, oriImageDivAirDark, Air);
    float *trans = (float *) malloc(sizeof(float) * LEN);
	writeImgF(width, height, oriImageDivAirDark, "wode——oriImageDivAirDark.jpg");
    GetTrans(oriImageDivAirDark, width, height, mRadius, trans, 0.95f);

	writeImgF(width, height, trans, "wodetoushe.jpg");
	//trans  255
    GuidedFilter(oriR, oriG, oriB, trans, width, height, mRadius*4, 0.1f);
	float *trans2 = (float *) malloc(sizeof(float) * LEN);
	for(int i = 0; i < 450*600;++i)
	{
		trans2[i] = trans[i] * 255;
	}
	writeImgF(width, height, trans2, "wodetoushe_refine.jpg");


    //以上，计算得到透射率
    /////////////////////////////////////////////////////////
    const float MIN_T = 0.1f;
    for (int i = 0, j = 0; i < 4 * LEN; i += 4, ++j) {
        float v = (imageDataRGBA[i] - Air[0]) / MAX(0.1f, trans[j]) + Air[0];
        imageDataRGBA[i] = CLAM(v);
        v = (imageDataRGBA[i + 1] - Air[1]) / MAX(0.1f, trans[j]) + Air[1];
        imageDataRGBA[i + 1] = CLAM(v);
        v = (imageDataRGBA[i + 2] - Air[2]) / MAX(0.1f, trans[j]) + Air[2];
        imageDataRGBA[i + 2] = CLAM(v);
    }
	cv::Mat myfine(height, width, CV_8UC4, imageDataRGBA);
	cv::imwrite("myfinal.jpg", myfine);

}

//out:输出A 的rgb值
void LyhDehazor::AirlightEsimation(unsigned char *oriImage, unsigned char *oriDark, unsigned char *out, int len, int low,
                                   int heigh) {
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
    //LOG("AirlightEsimation   begin");
    const float SCALE = 0.001f;
    int threshold;
    if (true) {
        const int m = (int) (heigh - (heigh - low) * SCALE);
        //LOG("AirlightEsimation my air  m:%d  low:%d   height:%d", m, low, heigh);
        int num = 0;
        for (int i = 0; i < len; ++i) {
            if (oriDark[i] >= m) {
                ++num;
            }
        }
        threshold = (int) (m + (num - SCALE * len) / num * (heigh - m));
        //LOG("AirlightEsimation my air, threshold:%d   num:%d", threshold , num);
    }

    if (true) {  //采用常规方法计算air
        //LOG("AirlightEsimation normal air");
        int maxIndex[256] = {0};
        for (int i = 0; i < len; ++i)  //统计0-255每个值的的数量分布
        {
            ++maxIndex[oriDark[i]];
        }
        int cnt = (int) (SCALE * len);
        for (threshold = 255; threshold >= 0; --threshold) //从最大的开始找到cnt个
        {
            if (cnt > 0) {
                cnt -= maxIndex[threshold];
            } else {
                break;
            }
        }
        //LOG("AirlightEsimation  normal air, threshold:%d", threshold);
    }


    int num = 0;
    int r = 0, g = 0, b = 0;
    for (int i = 0, j = 0; i < len ; ++i, j+=4) {
        if (oriDark[i] >= threshold) {
            r += oriImage[j];
            g += oriImage[j + 1];
            b += oriImage[j + 2];
            ++num;
        }
    }
    out[0] = r / num;
    out[1] = g / num;
    out[2] = b / num;
	out[0] = 197.23333;
	out[1] = 219.23333;
	out[2] = 230.51111;
    //LOG("AirlightEsimation end  %d %d  %d", (int)out[0],(int)out[1],(int)out[2]);
}

//对应公式12中的I(y)/A，即查找原图除以大气光之后的暗通道  . len = width*height
//输出之后的out值很小，属于归一化的值
void LyhDehazor::ImageDivAir(unsigned char *oriImage, int len, float *out, unsigned char *air) {
    float air_r = air[0], air_g = air[1], air_b = air[2];
    for (int i = 0, j = 0; j < len; ++j, i += 4) {
        out[j] = MINT(oriImage[i] / air_r, oriImage[i + 1] / air_g, oriImage[i + 2] / air_b);
    }
}




	/**
 * 以r为宽高，把范围内所有值合并到outdata中，然后再除以r*r,即使用box进行均值滤波
 * @param data
 * @param outdata
 * @param r  the radius of kernel,then the width and height is 2r+1
 * @param width
 * @param height
 * @param cumtype   对于T1为unsigned char则T2为int, float则为float.   这个参数的值没有用到
 */
template<class T1, class T2,class T3>
void LyhDehazor::MeanFilter(T1 *data, T3 *outdata, int r, int width, int height, T2 cumtype) {
    //LOG("MeanFilter  beging");
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
    for (int i = 0 ; i < (r + 1) * width; i += block)   //不需要担心end是不是block的整数倍，就算不是，超过的部分也会在后面重新计算正确的值
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
		//循环展开，每次处理四行。且每次从第二个元素开始（+1），因为第一列已经处理过了
        for (int i = y0 + 1; i < y1; ++i) {  //处理一行
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
			int a = mDivN[y * width + x];

            outdata[y * width + x] = cum[y * width + x + r] / mDivN[y * width + x]; //！！！！！！！！！mDivN[y * width + x]
			int b = outdata[y * width + x];
			int c = a+b;
        }
        for (int x = r + 1; x < width - r; ++x) {
            outdata[y * width + x] =
                    (cum[y * width + x + r] - cum[y * width + x - r - 1]) / mDivN[y * width + x];
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



//此函数的效果等于：A = data1 .* data2； 然后再MeanFilter(A)
template<class T1, class T2, class T3>
void LyhDehazor::MulMeanFilter(T1 *data1, T2 *data2, T3 *outdata, int r, int width, int height) {
    T3 *cum = (T3 *) malloc(sizeof(T3) * width * height);

	if (!cum) {
		//LOG("BoxFilter malloc failed!!!!!!");
		return;
	}
	/**********this  ringht this*************/

	const int len = width * height;
	const int block = 4; //

	//cum y
	for (int i = 0; i < width; ++i) {//the first row
		cum[i] = data1[i] * data2[i];
	}
	for (int i = width; i < len; i += width) {
		int end = i + width / block * block;
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
	const int R_WIDTH = r * width;
	const int R1_WIDTH = width * (r + 1);
	for (int i = 0 ; i < (r + 1) * width; i += block)   //不需要担心end是不是block的整数倍，就算不是，超过的部分也会在后面重新计算正确的值
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
		//循环展开，每次处理四行。且每次从第二个元素开始（+1），因为第一列已经处理过了
		for (int i = y0 + 1; i < y1; ++i) {  //处理一行
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
			outdata[y * width + x] = cum[y * width + x + r] /mDivN[y * width + x] ;///mDivN[y * width + x]
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

}

//此函数效果等于：out = input1 .* input2
template<class T1, class T2, class T3>
void LyhDehazor::ArrayMul(T1 *input1, T2 *input2, int len, T3 *out) {
    for (int i = 0; i < len; ++i) {
        out[i] = input1[i] * input2[i];
    }
}

//此函数效果等于：exy = exy - ex .* ey
template<class T1, class T2, class T3>
void LyhDehazor::Variance(T1 *exy, T2 *ex, T3 ey, int len) {
    for (int i = 0; i < len; ++i) {
        exy[i] = ex[i] - ey[i];
    }
}

template<class T>
void LyhDehazor::Normalized(T *data, float *out, int len)
{
    for(int i = 0; i < len; ++i)
    {
        out[i] = data[i] / 255.0f;
    }
}


//ori_r分别为原图的rgb通道， data为初步的透射率图,为0-255， 图片尺寸width*height  滤波半径size eps
void LyhDehazor::GuidedFilter(unsigned char *ori_r, unsigned char *ori_g, unsigned char *ori_b, float *data, int width,
                              int height, int size, float e)
{
	BoxDivN(mDivN, 600,450,size);
	writeImg(width, height, ori_r, "ori_r.jpg");
	writeImg(width, height, ori_r, "ori_b.jpg");
	writeImg(width, height, ori_r, "ori_g.jpg");

    int LEN = width * height;
    float *fr = (float *) malloc(sizeof(float) * LEN);
    float *fg = (float *) malloc(sizeof(float) * LEN);
    float *fb = (float *) malloc(sizeof(float) * LEN);
    Normalized(ori_r, fr, LEN);
    Normalized(ori_g, fg, LEN);
    Normalized(ori_b, fb, LEN);
	Normalized(data, data, LEN);

    float *mr = (float *) malloc(sizeof(float) * LEN);
    float *mg = (float *) malloc(sizeof(float) * LEN);
    float *mb = (float *) malloc(sizeof(float) * LEN);
    MeanFilter(fr, mr, size, width, height, 1.0f);
    MeanFilter(fg, mg, size, width, height, 1.0f);
    MeanFilter(fb, mb, size, width, height, 1.0f);

    float *md = (float *) malloc(sizeof(float) * LEN);
    MeanFilter(data, md, size, width, height, 1.0f);

	float *mdr = (float *) malloc(sizeof(float) * LEN);
	float *mdr2 = (float *) malloc(sizeof(float) * LEN);
    MulMeanFilter(data, fr, mdr, size, width, height);

    for (int i = 0; i < LEN; ++i) {
        mdr[i] -= md[i] * mr[i];
		mdr2[i] = mdr[i] *255*255;
    }
	writeImgF(width,height, mdr2, "my_d_r22222222222.jpg");


    float *mdg = (float *) malloc(sizeof(float) * LEN);
    MulMeanFilter(data, fg, mdg, size, width, height);
    for (int i = 0; i < LEN; ++i) {
        mdg[i] -= md[i] * mg[i];
    }

    float *mdb = (float *) malloc(sizeof(float) * LEN);
    MulMeanFilter(data, fb, mdb, size, width, height);
    for (int i = 0; i < LEN; ++i) {
        mdb[i] -= md[i] * mb[i];
    }

    float *mrr = (float *) malloc(sizeof(float) * LEN);
    MulMeanFilter(fr, fr, mrr, size, width, height);
    for (int i = 0; i < LEN; ++i) {
        mrr[i] -= (mr[i] * mr[i]);
    }

    float *mrg = (float *) malloc(sizeof(float) * LEN);
    MulMeanFilter(fr, fg, mrg, size, width, height);
    for (int i = 0; i < LEN; ++i) {
        mrg[i] -= (mr[i] * mg[i]);
    }

    float *mrb = (float *) malloc(sizeof(float) * LEN);
    MulMeanFilter(fr, fb, mrb, size, width, height);
    for (int i = 0; i < LEN; ++i) {
        mrb[i] -= (mr[i] * mb[i]);
    }

    float *mbb = (float *) malloc(sizeof(float) * LEN);
    MulMeanFilter(fb, fb, mbb, size, width, height);
    for (int i = 0; i < LEN; ++i) {
        mbb[i] -= (mb[i] * mb[i]);
    }

    float *mbg = (float *) malloc(sizeof(float) * LEN);
    MulMeanFilter(fb, fg, mbg, size, width, height);
    for (int i = 0; i < LEN; ++i) {
        mbg[i] -= (mb[i] * mg[i]);
    }

    float *mgg = (float *) malloc(sizeof(float) * LEN);
    MulMeanFilter(fg, fg, mgg, size, width, height);
    for (int i = 0; i < LEN; ++i) {
        mgg[i] -= (mg[i] * mg[i]);
    }


    float eps = 0.001f;//�ݶ�
    float *ar = (float *) malloc(sizeof(float) * LEN);
    float *ag = (float *) malloc(sizeof(float) * LEN);
    float *ab = (float *) malloc(sizeof(float) * LEN);
    float *b = (float *) malloc(sizeof(float) * LEN);
    for (int i = 0; i < LEN; ++i) {
		mrr[i] += eps;mbb[i] += eps; mgg[i]+=eps;
		//(rr*gb^2 - 2*gb*rb*rg + gg*rb^2 + bb*rg^2 - bb*gg*rr)
		float D = mrr[i]*(mgg[i]*mbb[i]-mbg[i]*mbg[i]) - mrg[i]*(mrg[i]*mbb[i]-mrb[i]*mbg[i]) + mrb[i]*(mrg[i]*mbg[i]-mrb[i]*mgg[i]);
        float v1 = (mgg[i]) * (mbb[i] ) - mbg[i] * mbg[i];  v1 /= D;
        float v2 = mbg[i] * mrb[i] - mrg[i] * (mbb[i] );  v2 /= D;
        float v3 = (mrr[i] ) * (mbb[i] ) - mrb[i] * mrb[i];  v3 /= D;
        float v4 = mrg[i] * mbg[i] - (mgg[i] ) * mrb[i];  v4 /= D;
        float v5 = mrg[i] * mrb[i] - (mrr[i] ) * mbg[i];  v5 /= D;
        float v6 = (mrr[i]) * (mgg[i]) - mrg[i] * mrg[i];  v6 /= D;

        ar[i] = mdr[i] * v1 + mdg[i] * v2 + mdb[i] * v4;
        ag[i] = mdr[i] * v2 + mdg[i] * v3 + mdb[i] * v5;
        ab[i] = mdr[i] * v4 + mdg[i] * v5 + mdb[i] * v6;
    }
    for (int i = 0; i < LEN; ++i) {
        b[i] = md[i] - ar[i] * mr[i] - ag[i] * mg[i] - ab[i] * mb[i];
    }

    delete [] mr;
    delete [] mg;
    delete [] mb;
    delete [] mrr;
    delete [] mrg;
    delete [] mrb;
    delete [] mbg;
    delete [] mgg;
    delete [] mbb;
    delete [] md;
    delete [] mdr;
    delete [] mdg;
    delete [] mdb;

    float *boxar = (float *) malloc(sizeof(float) * LEN);
    BoxFilter(ar, boxar, size, width, height, 1.0f);
    float *boxag = (float *) malloc(sizeof(float) * LEN);
    BoxFilter(ag, boxag, size, width, height, 1.0f);
    float *boxab = (float *) malloc(sizeof(float) * LEN);
    BoxFilter(ab, boxab, size, width, height, 1.0f);
    float *boxb = (float *) malloc(sizeof(float) * LEN);
    BoxFilter(b, boxb, size, width, height, 1.0f);


	int *nn = (int*)malloc(sizeof(int)*450*600);
	BoxDivN(nn, 600,450,size);


    for (int i = 0; i < LEN; ++i) {
        data[i] = (boxar[i] * fr[i] + boxag[i] * fg[i] + boxab[i] * fb[i] + boxb[i]) /
                  mDivN[i];
    }
}

//用来创建均值时的除数。因为图像边缘的不需要除以width*height
void LyhDehazor::BoxDivN(int *out, int width, int height, int r) {
  /*  int m, n, max;
    max = (2 * r + 1) * (2 * r + 1);   //这就是box整个都在图像范围内的情况，
    for (int x = 0, index = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            if (x >= r && x <= width - r - 1 && y >= r && y <= height - r - 1) {
                out[index++] = max;
            } else {
                m = MINT(r,x, width - 1 - x);
                n = MINT(r,y, height - 1 - y);
                out[index++] = (r + 1 + m) * (r + 1 + n);//对于边缘的
            }

        }
    }   */////////////////////////////
	int * input = (int *)malloc(sizeof(int)*width * height);
	for (int i = 0; i < width * height; ++i)
	{
		input[i] = 1;
	}
	BoxFilter(input, out,r,width,height, 1);
	for(int i = 0; i < 450*600;i+=100)
	{
		int a = out[i];
	}
}