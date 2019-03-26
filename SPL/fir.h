#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//	For a fir filter, if the order of fir filter is N, then the length of its
//	buffer size and coefficents is N+1.
typedef struct
{
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

FIR* newFir(unsigned int order, float *coeffs) {
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
FIR* createFir(unsigned int order, float* coeffs ) {

	FIR* obj = newFir(order, coeffs);
	return obj;
}
void stepFir(FIR*  obj, float* in,size_t* inOffset,float* out,size_t* outOffset, size_t stepNum) {

	// push data into the FIFO buffer
	// NOTE that in some cases,buffer_length is not equal to filter order
	// buffer_length must be a power of 2 >= filter order in order to use |&| operator.
	for (size_t stepCounter = 0; stepCounter < stepNum; stepCounter++)
	{
		obj->buffer[obj->bpos + stepCounter] = in[(*inOffset)++];
	}
	obj->bpos = (obj->bpos + stepNum) & (obj->bufferMask);		// update buffer pos

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
	out[(*outOffset)++] = sample;
}


static inline int runFir(FIR*  obj, float* in, float* out, size_t sampleNum,size_t stepNum) {
	size_t inOffset = 0;
	size_t outOffset = 0;
	if (sampleNum < stepNum)
	{
		return -1;
	}
	for (size_t n = 0; n < sampleNum; n+=stepNum)
	{
		stepFir(obj, in, &inOffset, out, &outOffset, stepNum);
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
	int bufferLen;
	int bufferMask;
	size_t splitBands_;
	size_t order_;

	int bpos;
	unsigned int symmetricOrder;		// the number of fir's symmetric coefficient 
	unsigned int tapNum;

	float **buffer;						// the buffer store fir state
	float **coeffs;						// half FIR coefficients
}
PolyphaseFilter;

PolyphaseFilter* newPolyphaseFilter(size_t order,size_t splitBands,float* coeffs) {
	// the number of ceofficients is filter order + 1
	// splitBands:	split the fir filter into |splitBands| bands

	PolyphaseFilter* obj = (PolyphaseFilter*)malloc(sizeof(PolyphaseFilter));
	obj->splitBands_ = splitBands;
	obj->order_ = order;
	obj->bufferLen =ceil((order + 1.0f) / (float)splitBands);

	obj->buffer =(float**)malloc(sizeof(float)*obj->splitBands_);
	obj->coeffs =(float**)malloc(sizeof(float)*obj->splitBands_);
	for (size_t n = 0; n < obj->splitBands_; n++)
	{
		obj->buffer[n] = (float*)malloc(sizeof(float)*obj->bufferLen);
		obj->coeffs[n] = (float*)malloc(sizeof(float)*obj->bufferLen);
		memset(obj->buffer[n], 0, sizeof(float)*obj->bufferLen);
		memset(obj->coeffs[n], 0, sizeof(float)*obj->bufferLen);
		for (size_t m = 0; m < obj->bufferLen; m++)
		{
			if (n*obj->bufferLen + m > order) 
			{
				break;
			}
			obj->coeffs[n][m] = coeffs[n*obj->bufferLen + m];
		}
	}

	return obj;
}
void runPolyphaseDecimation() {
	// up samplerate
}
void runPolyphaseInterpolation(PolyphaseFilter*obj,float in,float* interpOut,int interpNum) {
	// down samplerate

	for (size_t nBands = 0; nBands < obj->splitBands_; nBands++)
	{
		for (size_t n = 0; n < obj->bufferLen; n++)
		{
			// run subfir 
			interpOut[nBands];
		}
	}

}
void freePolyphaseFilter(PolyphaseFilter* obj) {
	if (obj)
	{
		for (size_t n = 0; n < obj->splitBands_; n++)
		{
			free(obj->buffer[n]);
			free(obj->coeffs[n]);
		}
		free(obj->coeffs);
		free(obj->buffer);
		free(obj);
	}
}