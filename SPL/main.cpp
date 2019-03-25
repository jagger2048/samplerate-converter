
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>

#include "response_measurement.h"
#include "biquad.h"
#include "fir.h"
#include <math.h>
using namespace std;

void irUnitTest() {

	float fs = 48e3;
	float f1 = 1;
	float f2 = 20e3;
	float duration = 1;
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
int main()
{
	//irUnitTest();
	firUnitTest();

	return 0;
}
