#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include "biquad.h"
#include "fir.h"
#include <math.h>
#include "src_fir_coeffs.h"

#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif // !M_PI

using namespace std;

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