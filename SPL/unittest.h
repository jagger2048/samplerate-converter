#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>

#include "response_measurement.h"
#include "biquad.h"
#include "fir.h"
#include <math.h>
#include "dynamic_bass_boost2.h"
#include "dynamic_bass_boost.h"
#include "response_measurement.h"

#include "src_fir_coeffs.h"
using namespace std;

void irUnitTest() {

	float fs = 48e3;
	float f1 = 1;
	float f2 = 20e3;
	float duration = 10;
	size_t len = duration * fs;
	float* ess = generateExpSineSweep(duration, f1, f2, fs);
	float* yFiltered = (float*)malloc(sizeof(float)*len);

	float coefs[2][3] = {
		{ 0.00391612666054739,	0.00783225332109477,	0.00391612666054739} ,
		{ 1, -1.81534108270457,	0.831005589346757 }
	};
	Biquad* filter = createBiquad(fs, coefs[0], coefs[1]);
	for (size_t n = 0; n < len; n++)
	{
		runBiquad(filter, ess[n], yFiltered[n]);
	}
	float* ir = (float*)malloc(sizeof(float)*len);
	float* mag_db = findSystemIR(yFiltered, duration, f1, f2, fs, ir);



	fstream fo;
	fo.open("biquad mag out.txt", ios::out);
	for (size_t n = 0; n < len + 1; n++)
	{
		fo << mag_db[n] << "\n";
	}
	fo.close();
	free(ess);
	free(yFiltered);
	free(mag_db);
	free(ir);
}

void firUnitTest() {
	
	// Symmetric fir, even and odd order
	// Asymmetric fir, even and odd order
	// Halfband fir,the symmetric part is even / odd
	// polyphase filter

	// even order
	float b1[55] = {

-0.000403295349090975,
- 0.000294262792808073,
- 0.000171586830118381,
1.95541765638554e-19,
0.000260242203866794,
0.000651405538885519,
0.00121606223142803,
0.00199472238892183,
0.00302342809326625,
0.00433143220351745,
0.00593908327002069,
0.00785603108450163,
0.0100798545015572,
0.0125951948243987,
0.0153734551014653,
0.0183730992313118,
0.0215405561176125,
0.0248117046809868,
0.0281138868034281,
0.0313683687182514,
0.0344931483312556,
0.0374059876665963,
0.0400275370413092,
0.0422844113696104,
0.0441120795453717,
0.0454574351705473,
0.0462809306666518,
0.0465581763745098,
0.0462809306666518,
0.0454574351705473,
0.0441120795453717,
0.0422844113696104,
0.0400275370413092,
0.0374059876665963,
0.0344931483312556,
0.0313683687182514,
0.0281138868034281,
0.0248117046809868,
0.0215405561176125,
0.0183730992313118,
0.0153734551014653,
0.0125951948243987,
0.0100798545015572,
0.00785603108450163,
0.00593908327002069,
0.00433143220351745,
0.00302342809326625,
0.00199472238892183,
0.00121606223142803,
0.000651405538885519,
0.000260242203866794,
1.95541765638554e-19,
- 0.000171586830118381,
- 0.000294262792808073,
- 0.000403295349090975 };	// b1 = fir1(54,1000/(48e3/2));
	// odd order
	float b2[54] = {
		-0.000348089297380851,
	- 0.000228410463389213,
	- 8.89309070185504e-05,
	0.000108606257913900,
	0.000406694189183810,
	0.000849893349988352,
	0.00148241487659708,
	0.00234557435118651,
	0.00347524458851981,
	0.00489944006260671,
	0.00663616275647145,
	0.00869162953454823,
	0.0110589849972099,
	0.0137175819131220,
	0.0166328847836085,
	0.0197570221814113,
	0.0230299817260349,
	0.0263814095277775,
	0.0297329453028359,
	0.0330009967272845,
	0.0360998334125758,
	0.0389448633852794,
	0.0414559440894746,
	0.0435605763144943,
	0.0451968333247697,
	0.0463158886870929,
	0.0468840243278017,
	0.0468840243278017,
	0.0463158886870929,
	0.0451968333247697,
	0.0435605763144943,
	0.0414559440894746,
	0.0389448633852794,
	0.0360998334125758,
	0.0330009967272845,
	0.0297329453028359,
	0.0263814095277775,
	0.0230299817260349,
	0.0197570221814113,
	0.0166328847836085,
	0.0137175819131220,
	0.0110589849972099,
	0.00869162953454823,
	0.00663616275647145,
	0.00489944006260671,
	0.00347524458851981,
	0.00234557435118651,
	0.00148241487659708,
	0.000849893349988352,
	0.000406694189183810,
	0.000108606257913900,
	- 8.89309070185504e-05,
	- 0.000228410463389213,
	- 0.000348089297380851,
			}; //b2 = fir1(53, 1000 / (48e3 / 2));
	// halfband odd / even 

	// test signal generate:
	float f1 = 500;
	float f2 = 2000;
	float fs = 48e3;
	float* x = (float*)malloc(sizeof(float)*fs);
	for (size_t t = 0; t < fs; t++)
	{
		//x[t] = 0.5*sin(2 * M_PI*f1*t / fs) + 0.5*sin(2 * M_PI*f2*t / fs);
		x[t] = t;
	}

	printf("=== FIR filter Test ===\n");
	FIR* firEven = createFir(53, b2,true);	// b1 54  b2 53
	//FIR* firEven = createFir(9, b9);
	float out[48000]{};

	runFir(firEven, x, out, 100, 5);

	//for (size_t n = 0; n < fs/10; n++)
	//{
	//	printf("%d : %f \n", n,out[n]);
	//}
	//fstream fo;
	//fo.open("b2 asymmetry odd step 10.txt", ios::out);
	//for (size_t n = 0; n < fs; n++)
	//{
	//	fo << x[n] << "	" << out[n] << "\n";
	//}
	//fo.close();

	//free(x);
	freeFir(firEven);

}


//void dbbUnitTest() {
//	// This is the DBB algorithm test.
//	// generate exponental sine sweep signal 1~20kHz,duration 1 s
//	float mag[9] = { -30,-24,-18,-15,-12,-9,-6 ,-3,0 };
//	float fs = 48e3;
//	float f1 = 1;
//	float f2 = 20e3;
//	float duration = 1;
//	size_t len = duration * fs;
//	float* ess = generateExpSineSweep(duration, f1, f2, fs);
//	float* essProcessed[9];
//	for (size_t i = 0; i < 9; i++)
//	{
//		essProcessed[i] = (float*)malloc(sizeof(float)*len);
//	}
//
//	DBB* dbb = createDBB(fs);
//
//	for (size_t num = 0; num < 9; num++)
//	{
//		for (size_t n = 0; n < len; n++)
//		{
//			runDBB(dbb, powf(10.0f, mag[num] / 20.0f)*ess[n], essProcessed[num][n]);
//		}
//	}
//
//	float* ir = (float*)malloc(sizeof(float)*len);
//	float* ir_mag[9]{};
//	for (size_t num = 0; num < 9; num++)
//	{
//		ir_mag[num] = findSystemIR(essProcessed[num], duration, f1, f2, fs, ir);
//
//	}
//
//	// output the response of the DBB system
//	fstream fo;
//	fo.open("dbb mag full scale.txt", ios::out);
//
//	for (size_t n = 0; n < len; n++)
//	{
//		for (size_t num = 0; num < 9; num++)
//		{
//			fo << ir_mag[num][n] << "	";
//		}
//		fo << "\n";
//	}
//	fo.close();
//	free(ess);
//	free(ir);
//	freeDBB(dbb);
//
//	for (size_t n = 0; n < 9; n++)
//	{
//		free(ir_mag[n]);
//		free(essProcessed[n]);
//	}
//
//}
void dbb2UnitTest_refactor() {
	// generate exponental sine sweep signal
	float mag[9] = { -30,-24,-18,-15,-12,-9,-6 ,-3,0 };
	float fs = 48e3;
	float f1 = 1;
	float f2 = 20e3;
	float duration = 10;
	size_t len = duration * fs;
	float* ess = generateExpSineSweep(duration, f1, f2, fs);

	float* essProcessed[9];
	for (size_t i = 0; i < 9; i++)
	{
		essProcessed[i] = (float*)malloc(sizeof(float)*len);
	}

	// your processing here
	float boostFreq = 60;
	float boostGain = 16;
	DBB2* dbb = createDbb2(boostFreq, boostGain, 1.5, fs);

	for (size_t num = 0; num < 9; num++)
	{
		for (size_t n = 0; n < len; n++)
		{
			runDbb2(dbb, powf(10.0f, mag[num] / 20.0f)*ess[n], essProcessed[num][n]);
		}
	}


	freeDbb2(dbb);
	//

		// find the impulse response of DBB
	float* ir = (float*)malloc(sizeof(float)*len);
	float* ir_mag[9];
	for (size_t num = 0; num < 9; num++)
	{
		ir_mag[num] = findSystemIR(essProcessed[num], duration, f1, f2, fs, ir);
	}
	fstream fo;
	fo.open("dbb2 mag -30 to 0 step 3.txt", ios::out);
	for (size_t n = 0; n < len / 2 + 1; n++)
	{
		for (size_t num = 0; num < 9; num++)
		{
			fo << ir_mag[num][n] << "	";
		}
		fo << "\n";
	}
	fo.close();
	free(ir);
	free(ess);
	for (size_t n = 0; n < 9; n++)
	{
		free(ir_mag[n]);
		free(essProcessed[n]);
	}


}
float* sineGenerator(float freq, float duration, float amplitude, float samplerate) {
	float* sine = NULL;
	sine = (float*)malloc(sizeof(float)*duration*samplerate);
	for (size_t t = 0; t < duration*samplerate; t++)
	{
		sine[t] = amplitude*sin(2.0f*M_PI*freq*float(t) / samplerate);
	}
	return sine;
}
void output2File(const char * fileName, float* data,size_t length) {
	fstream fo;
	fo.precision(15);
	fo.open(fileName, ios::out);
	for (size_t n = 0; n < length; n++)
	{

		fo << data[n] << "	";
		fo << "\n";
	}
	fo.close();
}
void polyphaseFilterUnittest() {
	

	float* x96 = sineGenerator(1000, 1, 1, 96000);
	PolyphaseFilter* p = newPolyphaseFilter(FIR_96_48, 2, FIR_96_48_24);
	FIR* f96 = createFir(FIR_96_48, FIR_96_48_24, false);
	float* y96 = (float*)malloc(sizeof(float) * 96000);
	// test case 1: 96k->48k	|	2:1
	// Use the polyphase decimation,M		|| test pass
	for (size_t n = 0; n < 96000; n+=2)
	{
		runPolyphaseDecimation(p, x96+n, y96+n/2, 2);
	}
	output2File("96 to 48.txt", y96, 96000 / 2);
	output2File("96 to 48 - 96.txt", x96, 96000);

	// test case 2: 48k->96k	|	1:2
	// Use the polyphase interpolation,L	|| test pass
	memset(y96, 0, sizeof(float) * 96000);

	float* x48 = sineGenerator(1000, 1, 1, 48e3);
	for (size_t n = 0; n < 48000; n++)
	{
		runPolyphaseInterpolation(p, x48 + n, y96 + n*2, 2);

	}
	output2File("48 to 96.txt", y96, 96000);
	output2File("48 to 96 - 48.txt", x48, 48000);

	free(x96);
	free(y96);

	// tese case 3: 96k->44.1	|	147:320
	// Use polyphase decimation and polyphase interpolation, M/L
	//runPolyphaseDecimation(p, in, out, 147);
	//runPolyphaseInterpolation(p, in, out, 320);

	// test case 4: 96k->44.1k	|	147:320
	// Use frational polyphase decimation and interpolation
	// To be complemented

	// test case 5: 96k->44.1	|	8:7->8:3->5:7
	// Use multi-state frational polyphase decimation and interpolation
	// To be complemented

	freePolyphaseFilter(p);
}