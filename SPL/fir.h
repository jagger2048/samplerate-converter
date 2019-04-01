#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//	For a fir filter, if the order of fir filter is N, then the length of its
//	buffer size and coefficents is N+1.
typedef struct
{
	// the stcuct of the fir filter, contains the coefficient symmetric fir and 
	// coefficient asymmetric fir filter.
	bool isSymmetry_;
	int bufferSize;
	int bufferMask;
	int bpos;
	unsigned int symmetricOrder;		// the number of fir's symmetric coefficient 
	unsigned int tapNum;	
	bool isEvenOrder;

	float *buffer;						// the buffer store fir state
	float *coeffs;						// half FIR coefficients
}
FIR;

FIR* newSymmetricFir(unsigned int order,const float *coeffs) {
	//	order:	FIR filter order
	//	coeffs: the coefficeents of FIR filter
	FIR* obj = (FIR *)malloc(sizeof(FIR));
	if (!obj)
	{
		return NULL;
	}
	obj->tapNum = order + 1;

	if (order % 2 == 0)
	{
		obj->isEvenOrder = true;
		obj->symmetricOrder = order / 2;		
	}
	else
	{
		obj->isEvenOrder = false;
		obj->symmetricOrder = (order+1) / 2;
	}

	int k = 1;
	while (k < order+1)
	{
		k <<= 1;
	}

	obj->bufferSize = k;
	obj->bufferMask = k - 1;
	obj->bpos = 0;

	obj->buffer = (float *)malloc(sizeof(float) * obj->bufferSize);
	obj->coeffs = (float *)malloc(sizeof(float) * (obj->tapNum - obj->symmetricOrder));

	memset(obj->buffer, 0, sizeof(float) * obj->bufferSize);
	memcpy(obj->coeffs, coeffs, sizeof(float) * (obj->tapNum - obj->symmetricOrder));
	return obj;
}

FIR* newAsymmetricFir(unsigned int order,const float *coeffs) {
	//	order:	FIR filter order
	//	coeffs: the coefficeents of FIR filter
	FIR* obj = (FIR *)malloc(sizeof(FIR));
	if (!obj)
	{
		return NULL;
	}
	obj->tapNum = order + 1;
	obj->symmetricOrder = 0;

	int k = 1;
	while (k < obj->tapNum)
	{
		k <<= 1;
	}
	obj->bpos = 0;
	obj->bufferSize = k;
	obj->bufferMask = k - 1;
	obj->coeffs = (float *)malloc(sizeof(float) * obj->tapNum );
	obj->buffer = (float *)malloc(sizeof(float) * obj->bufferSize);
	memset(obj->buffer, 0, sizeof(float) * obj->bufferSize);
	memcpy(obj->coeffs, coeffs, sizeof(float) * obj->tapNum );

	return obj;
}


static inline void stepSymmetricFir(FIR*  obj, float* in,float* out,size_t stepNum) {
	size_t pushIndex = 0;
	size_t popIndex = 0;
	// push data into the FIFO buffer
	// NOTE that in some cases,buffer_length is not equal to filter order
	// buffer_length must be a power of 2 >= filter order in order to use |&| operator.
	for (size_t stepCounter = 0; stepCounter < stepNum; stepCounter++)
	{
		if (obj->bpos + stepCounter > obj->bufferMask)
		{
			printf("overflow: %d - %d \n", obj->bpos, obj->bufferMask);
			break;
		}
		obj->buffer[obj->bpos + stepCounter] = in[pushIndex++];	// 测试 在 stepNum 不为 2 的倍数时是否会导致溢出
	}

	obj->bpos = (obj->bpos + stepNum) & (obj->bufferMask);		// update buffer pos
	//if (obj->bpos == obj->bufferMask)
	//{
	//	obj->bpos = 0;
	//}

	float sample = 0;
	int rr = obj->bpos;											// right seek index
	int ll = (obj->bpos + obj->tapNum - 1) & obj->bufferMask;	// left seek index	% obj->bufferMask;
	
	int mm = (obj->bpos + obj->symmetricOrder) & obj->bufferMask; // middle index for even order FIR

	for (size_t kk = 0; kk < obj->symmetricOrder; kk++)
	{
		//sample += obj->coeffs[kk] * (obj->buffer[(rr++)&obj->bufferMask] + obj->buffer[(ll--)&obj->bufferMask]);
		// use |+ / - kk| to replace |++ -- | operate in order to ovaid |ll rr| overflow.
		sample += obj->coeffs[kk] * (
			obj->buffer[(rr + kk)&obj->bufferMask] +
			obj->buffer[(ll - kk)&obj->bufferMask]);
	}
	if (obj->isEvenOrder)
	{
		//sample += 0.5f * obj->buffer[mm]; for half band fir filter
		sample += obj->coeffs[obj->symmetricOrder] * obj->buffer[mm];
	}
	out[popIndex++] = sample;
}

//static inline void stepAsymmetricFir(FIR*  obj, float* in, size_t* inOffset, float* out, size_t* outOffset, size_t stepNum) {
static inline void stepAsymmetricFir(FIR*  obj, float* in,  float* out, size_t stepNum) {

	size_t pushIndex = 0;
	size_t popIndex = 0;

	// push data into the FIFO buffer
	// NOTE that in some cases,buffer_length is not equal to filter order
	// buffer_length must be a power of 2 >= filter order in order to use |&| operator.
	for (size_t stepCounter = 0; stepCounter < stepNum; stepCounter++)
	{
		obj->buffer[obj->bpos + stepCounter] = in[pushIndex++];
	}
	obj->bpos = (obj->bpos + stepNum) & (obj->bufferMask);		// update buffer pos

	float sample = 0;
	size_t kk = 0;
	for (; kk < obj->tapNum; kk++)
	{
		sample += obj->coeffs[kk] * obj->buffer[(obj->bpos + kk)&obj->bufferMask] ;
	}
	out[popIndex++] = sample;
}

FIR* createHalfbandFir(unsigned int order, float *coeffs) {
	//	Hanfband Fir is a set of Symmetry FIR, and its coefficients at even order is 0.
	//	order:	FIR filter order
	//	coeffs: the coefficeents of FIR filter
	FIR* obj = (FIR *)malloc(sizeof(FIR));
	if (!obj)
	{
		return NULL;
	}
	obj->tapNum = order + 1;


	obj->isEvenOrder = false;
	obj->symmetricOrder = (order + 1) / 2;

	int k = 1;
	while (k < order + 1)
	//while (k < obj->symmetricOrder + 1)
	{
		k <<= 1;
	}

	obj->bufferSize = k;
	obj->bufferMask = k - 1;
	obj->bpos = 0;

	obj->buffer = (float *)malloc(sizeof(float) * obj->bufferSize);
	obj->coeffs = (float *)malloc(sizeof(float) * obj->symmetricOrder);

	memset(obj->buffer, 0, sizeof(float) * obj->bufferSize);

	for (size_t n = 0; n < (obj->symmetricOrder-1) /2; n++)
	{
		obj->coeffs[n] = coeffs[n * 2 + 1];
	}
	obj->coeffs[obj->symmetricOrder - 1] = coeffs[obj->symmetricOrder];// 0.5
	memcpy(obj->coeffs, coeffs, sizeof(float) * (obj->tapNum - obj->symmetricOrder));
	return obj;
}

static inline void stepHalfbandFir(FIR*  obj, float* in, float* out, size_t stepNum) {
	size_t pushIndex = 0;
	size_t popIndex = 0;

	// push data into the FIFO buffer
	// NOTE that in some cases,buffer_length is not equal to filter order
	// buffer_length must be a power of 2 >= filter order in order to use |&| operator.
	for (size_t stepCounter = 0; stepCounter < stepNum; stepCounter++)
	{
		obj->buffer[obj->bpos + stepCounter] = in[pushIndex++];
	}
	obj->bpos = (obj->bpos + stepNum) & (obj->bufferMask);		// update buffer pos

	float sample = 0;
	int rr = obj->bpos;											// right seek index
	int ll = (obj->bpos + obj->tapNum - 1) & obj->bufferMask;	// left seek index	% obj->bufferMask;

	int mm = (obj->bpos + obj->symmetricOrder) & obj->bufferMask; // middle index for even order FIR

	for (size_t kk = 0; kk < obj->symmetricOrder/2; kk++)
	{
		//sample += obj->coeffs[kk] * (obj->buffer[(rr++)&obj->bufferMask] + obj->buffer[(ll--)&obj->bufferMask]);
		// use |+ / - kk| to replace |++ -- | operate in order to ovaid |ll rr| overflow.
		sample += obj->coeffs[kk] * (
			obj->buffer[(rr + kk*2 + 1)&obj->bufferMask] +
			obj->buffer[(ll - kk*2 - 1)&obj->bufferMask]);
	}

	//sample += 0.5f * obj->buffer[mm]; for half band fir filter
	sample += obj->coeffs[obj->symmetricOrder] * obj->buffer[mm];

	out[popIndex++] = sample;
}

static inline int runHalfbandFir(FIR*  obj, float* in, float* out, size_t sampleNum, size_t stepNum) {
	size_t inOffset = 0;
	size_t outOffset = 0;
	if (sampleNum < stepNum)
	{
		return -1;
	}
	for (size_t n = 0; n < sampleNum; n += stepNum)
	{

		stepHalfbandFir(obj, in + n, out + n / stepNum, stepNum);
	}
	return 0;
}
FIR* createFir(unsigned int order, const float* coeffs, bool isSymmetry) {

	FIR* obj;
	if (isSymmetry)
	{
		obj = newSymmetricFir(order, coeffs);
		obj->isSymmetry_ = isSymmetry;

	}
	else
	{
		obj = newAsymmetricFir(order, coeffs);
		obj->isSymmetry_ = isSymmetry;

	}
	return obj;
}

static inline int runFir(FIR*  obj, float* in, float* out, size_t sampleNum,size_t stepNum) {
	// Note that the stepNum must a power of 2

	size_t inOffset = 0;
	size_t outOffset = 0;
	if (sampleNum < stepNum)
	{
		return -1;
	}
	for (size_t n = 0; n < sampleNum/ stepNum * stepNum; n+=stepNum)
	{
		if (obj->isSymmetry_)
		{
			//stepSymmetricFir(obj, in, &inOffset, out, &outOffset, stepNum);
			stepSymmetricFir(obj, in+n,out + n/stepNum,stepNum);
		}
		else
		{
			stepAsymmetricFir(obj, in+n, out+n/stepNum, stepNum);
		}
	}
	return 0;
}
void freeFir(FIR* obj) {
	if (obj)
	{
		free(obj->buffer);
		free(obj->coeffs);
		free(obj);
	}
}


// Polyphase filter
typedef struct 
{
	//int bandsNum;
	//int bufferMask;
	size_t splitBands_;
	size_t order_;


	//unsigned int symmetricOrder;		// the number of fir's symmetric coefficient 
	//unsigned int tapNum;

	// below parameters will be replace be the |firBands|
	float *buffer;						// the buffer store fir state
	size_t bufferLen;
	size_t bufferMask;
	int bpos;

	float **coeffs;						// half FIR coefficients

	FIR** firBands;		// the number of firBands is splitBands
	size_t bandsLen;
}
PolyphaseFilter;

PolyphaseFilter* newPolyphaseFilter(size_t order,size_t splitBands,const float* coeffs) {
	// the number of ceofficients is filter order + 1
	// splitBands:	split the fir filter into |splitBands| bands

	PolyphaseFilter* obj = (PolyphaseFilter*)malloc(sizeof(PolyphaseFilter));
	obj->splitBands_ = splitBands;
	obj->order_ = order;
	obj->bandsLen =ceil((order + 1.0f) / (float)splitBands); // bands buffer length
	obj->bufferLen = 1;
	while (obj->bufferLen < obj->splitBands_)
	{
		obj->bufferLen <<= 1;
	}
	obj->buffer = (float*)malloc(sizeof(float)*obj->bufferLen);
	memset(obj->buffer, 0, sizeof(float)*obj->bufferLen);
	obj->bufferMask = obj->bufferLen - 1;
	obj->bpos = 0;

	// initialize every fir bands
	obj->firBands = (FIR**)malloc(sizeof(FIR)*obj->splitBands_);
	obj->coeffs =(float**)malloc(sizeof(float)*obj->splitBands_);
	for (size_t nBands = 0; nBands < obj->splitBands_; nBands++)
	{
		obj->coeffs[nBands] = (float*)malloc(sizeof(float)*obj->bandsLen);
		memset(obj->coeffs[nBands], 0, sizeof(float)*obj->bandsLen);

		size_t m = 0;
		for (; m < obj->bandsLen; m++)
		{
			//if (nBands*obj->bandsLen + m > order)
			if (obj->splitBands_ * m + nBands > order+1)
			{
				break;
			}
			obj->coeffs[nBands][m] = coeffs[obj->splitBands_ * m + nBands];
		}
		
		obj->firBands[nBands] = newAsymmetricFir(m - 1, obj->coeffs[nBands]);
		// 可以考虑把 coeffs free 掉
	}

	return obj;
}
void runPolyphaseDecimation(PolyphaseFilter* obj, float* in, float* decimatedOut, size_t decimatedNum) {
	// M
	// down samplerate, input |decimatedNum| samples, output one sample
	float tmp = 0;
	float sum = 0;

	for (size_t nBands = 0; nBands < obj->splitBands_; nBands++)
	{
		stepAsymmetricFir(obj->firBands[nBands], in+ obj->splitBands_ - nBands - 1, &tmp, 1);
		sum += tmp;
	}
	*decimatedOut = sum;
}
void runPolyphaseInterpolation(PolyphaseFilter* obj,float* in,float* interpolatedOut, size_t interpolatedNum) {
	// L
	//  up samplerate, input one sample, output |interpolatedNum| samples

	for (size_t nBands = 0; nBands < obj->splitBands_; nBands++)
	{
		stepAsymmetricFir(obj->firBands[nBands], in, interpolatedOut + obj->splitBands_ - nBands -1, 1);
	}

}
void freePolyphaseFilter(PolyphaseFilter* obj) {
	if (obj)
	{
		for (size_t n = 0; n < obj->splitBands_; n++)
		{
			free(obj->coeffs[n]);
			freeFir(obj->firBands[n]);
			obj->coeffs[n] = NULL;
			obj->firBands[n] = NULL;
		}
		free(obj->coeffs);
		free(obj->buffer);
		free(obj);
		obj->coeffs = NULL;
		obj->buffer = NULL;
		obj = NULL;
	}
}