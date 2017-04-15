#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include "JniEnvInit.h"
#include <pthread.h>

#define UCHAR unsigned char
#define DEBUG false
class LyhDehazor
{

private:
	int mWidth;
	int mHeight;
    const static int RADIUS = 7;
    int * mDivN;
    int mRadius;
    template<class T> void Normalized(T *data, float *out, int len);
    void AirlightEsimation(unsigned char *oriImage, unsigned char *oriDark, unsigned char *out, int len, int low, int heigh);
    void ImageDivAir(unsigned char *oriImage, int len, float *out, unsigned char *air);
    template<class T1, class T2, class T3> void MulMeanFilter(T1 *data1, T2 *data2, T3 *outdata, int r, int width, int height);
    template<class T1, class T2, class T3> void ArrayMul(T1 *input1, T2 *input2, int len, T3 *out);
    template<class T1, class T2, class T3> void Variance(T1 *exy, T2 *ex, T3 ey, int len);
    void GuidedFilter(unsigned char *guidedImage, float *data, int width, int height, int size, int e);
    void GuidedFilter(unsigned char *ori_r, unsigned char *ori_g,unsigned char *ori_b, float *data, int width, int height,int size,float e);
    void BoxDivN(int *out, int width, int height, int r);

    void MinFilter(unsigned char *data, int width, int height, int boxWidth, int boxHeight, unsigned char *out);
    void GetTrans(float *data, int width, int height, int boxsize, float *out, float w);
    unsigned char MinLine(unsigned char *data, int width, int line, int left, int right);
public:

    void grayGuidFilter(cv::Mat gray, cv::Mat trans, int size, float eps);

    template<class T1, class T2,class T3> void MeanFilter(T1 *data, T3 *outdata, int r, int width, int height , T2 cumtype);

	    LyhDehazor(int width, int height, int r);
    ~LyhDehazor(void);    
    float MinLine(float *data, int width, int line, int left, int right);
    void Dehazor(unsigned char *imageDataRGBA, int width, int height);
	static void writeImg(int width, int height, unsigned char *data, std::string name)
	{
		cv::Mat img(height, width, CV_8UC1, data);
		cv::imwrite(name, img);
	}
	static void writeImgF(int width, int height, float *data, std::string name)
	{
		cv::Mat img(height, width, CV_32FC1, data);
		cv::imwrite(name, img);
	}

	template<class T1, class T2>
	void BoxFilter(T1 *data, T2 *outdata, int r, int width, int height, T2 t) {
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
		for (int i = 0 ; i < (r + 1) * width; i += block)   //不要担心end是不是bolck的整数倍，就算不是，超过的部分也会在后面重新计算正确的值
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
			//循环展开，每次处理四行，且每次从第二个元素开始（+1），因为第一列已经处理过了
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

};

