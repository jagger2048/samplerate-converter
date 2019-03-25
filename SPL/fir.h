#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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