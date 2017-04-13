#pragma once

#include "JniEnvInit.h"

#define INPUT_NULL -1

class FastDehazor
{
public:
	FastDehazor(int width, int height);
	~FastDehazor();
	int process(unsigned char * rgba, int width, int height);
protected:
private:
	int * mDivN;
	int mWinSize;
	float mUpper;
	float mP;
	unsigned char * mResultTable;
	void InitResultTable();
	//在算法中，air的三个通道的值都是一样的
	unsigned char mAir;
	void MeanFilter(unsigned char *data, int * outdata, int r, int width, int height);
	void BoxDivN(int *out, int width, int height, int r);
	template<class T1, class T2> void BoxFilter(T1 *data, T2 *outdata, int r, int width, int height, T2 t);
};