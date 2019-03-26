
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
		{ 1, - 1.81534108270457,	0.831005589346757 }
	};
	Biquad* filter = createBiquad(fs,coefs[0],coefs[1]);
	for (size_t n = 0; n < len; n++)
	{
		runBiquad(filter, ess[n], yFiltered[n]);
	}
	float* ir = (float*)malloc(sizeof(float)*len);
	float* mag_db =  findSystemIR(yFiltered, duration, f1,f2, fs,ir);
	


	fstream fo;
	fo.open("biquad mag out.txt", ios::out);
	for (size_t n = 0; n < len+1 ; n++)
	{
		fo << mag_db[n]<< "\n";
	}
	fo.close();
	free(ess);
	free(yFiltered);
	free(mag_db);
	free(ir);
}

void firUnitTest() {
	float b8[9] = {
		0.0175031669762076,
		0.0479434791902531,
		0.122314130002404,
		0.197681263858199,
		0.229115919945871,
		0.197681263858199,
		0.122314130002404,
		0.0479434791902531,
		0.0175031669762076 };	// b8 = fir1(8,1000/(48e3/2));

	float b9[10] = {
		0.0154291143803247,
		0.0370373965011290,
		0.0924180054346794,
		0.156444301487403,
		0.198671182196464,
		0.198671182196464,
		0.156444301487403,
		0.0924180054346794,
		0.0370373965011290,
		0.0154291143803247 }; //b9 = fir1(9, 1000 / (48e3 / 2));


	// test signal generate:
	float f1 = 100;
	float f2 = 2000;
	float fs = 48e3;
	float* x = (float*)malloc(sizeof(float)*fs);
	for (size_t t = 0; t < fs; t++)
	{
		x[t] = 0.5*sin(2 * M_PI*f1*t / fs) + 0.5*sin(2 * M_PI*f2*t / fs);
	}


	printf("=== FIR filter Test ===\n");
	FIR* firEven = createFir(8, b8);
	//FIR* firEven = createFir(9, b9);
	float out[48000]{};

	runFir(firEven, x, out, fs, 1);

	fstream fo;
	fo.open("1000 Hz lpf.txt", ios::out);
	for (size_t n = 0; n < fs; n++)
	{
		fo <<x[n]<<"	"<< out[n] << "\n";
	}
	fo.close();

	free(x);
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

void polyphaseFilterUnittest() {
	float coeffs[10];
	for (size_t i = 0; i < 10; i++)
	{
		coeffs[i] = i;
	}
	PolyphaseFilter* p = newPolyphaseFilter(4, 2, coeffs);


	for (size_t n = 0; n < p->splitBands_; n++)
	{
		for (size_t j = 0; j < p->bufferLen; j++) {
			//printf(" %f - %f ", p->buffer[n][j], p->coeffs[n][j]);
			printf(" %f ",p->coeffs[n][j]);
		}
		printf("\n");
		printf("\n");
	}

	freePolyphaseFilter(p);
}
int main()
{
	//irUnitTest();
	////dbb2UnitTest_refactor();
	//firUnitTest();
	polyphaseFilterUnittest();
	return 0;
}




