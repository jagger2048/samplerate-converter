#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <fstream>
#include "response_measurement.h"
#include "dynamic_bass_boost.h"
#include"dynamic_bass_boost2.h"
using namespace std;
void dbbUnitTest() {
	// This is the DBB algorithm test.
	// generate exponental sine sweep signal 1~20kHz,duration 1 s
	float mag[9] = { -30,-24,-18,-15,-12,-9,-6 ,-3,0 };
	float fs = 48e3;
	float f1 = 1;
	float f2 = 20e3;
	float duration = 1;
	size_t len = duration * fs;
	float* ess = generateExpSineSweep(duration, f1, f2, fs);
	float* essProcessed[9];
	for (size_t i = 0; i < 9; i++)
	{
		essProcessed[i] = (float*)malloc(sizeof(float)*len);
	}

	DBB* dbb = createDbb(fs);

	for (size_t num = 0; num < 9; num++)
	{
		for (size_t n = 0; n < len; n++)
		{
			runDbb(dbb, powf(10.0f, mag[num] / 20.0f)*ess[n], essProcessed[num][n]);
		}
	}

	float* ir = (float*)malloc(sizeof(float)*len);
	float* ir_mag[9]{};
	for (size_t num = 0; num < 9; num++)
	{
		ir_mag[num] = findSystemIR(essProcessed[num], duration, f1, f2, fs, ir);

	}

	// output the response of the DBB system
	fstream fo;
	fo.open("dbb mag full scale.txt", ios::out);

	for (size_t n = 0; n < len; n++)
	{
		for (size_t num = 0; num < 9; num++)
		{
			fo << ir_mag[num][n] << "	";
		}
		fo << "\n";
	}
	fo.close();
	free(ess);
	free(ir);
	freeDbb(dbb);

	for (size_t n = 0; n < 9; n++)
	{
		free(ir_mag[n]);
		free(essProcessed[n]);
	}

}
void dbb2UnitTest_refactor() {
	// generate exponental sine sweep signal
	float mag[9] = { -30,-24,-18,-15,-12,-9,-6 ,-3,0 };
	float fs = 48e3;
	float f1 = 1;
	float f2 = 20e3;
	float duration = 1;
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
