#pragma once
#include "FFT.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <complex>
#include <algorithm>
#include <memory>
#include <iostream>
#include <thread>
#include <future>

#include <pmmintrin.h>

using std::complex;

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

std::vector<my_complex*> twiddle_table;
void fft_ensure_table(int k) {
	//  Makes sure the twiddle factor table is large enough to handle an FFT of
	//  size 2^k.

	int current_k = (int)twiddle_table.size() - 1;
	if (current_k >= k)
		return;

	//  Do one level at a time
	if (k - 1 > current_k) {
		fft_ensure_table(k - 1);
	}

	size_t length = (size_t)1 << k;
	double omega = 2 * M_PI / length;
	length /= 2;

	//  Build the sub-table.
	my_complex* sub_table = new my_complex[length];
	for (size_t c = 0; c < length; c++) {
		//  Generate Twiddle Factor
		double angle = omega * c;
		auto twiddle_factor = my_complex(cos(angle), sin(angle));
		sub_table[c] = twiddle_factor;
	}

	//  Push into main table.
	twiddle_table.push_back(sub_table);
}



void fft_forward(__m128d* T, int k, int threads) {
	//  Fast Fourier Transform
	//  This function performs a forward FFT of length 2^k.

	//  This is a Decimation-in-Frequency (DIF) FFT.
	//  The frequency domain output is in bit-reversed order.

	//Parameters:
	//  -   T           -   Pointer to array.
	//  -   k           -   2^k is the size of the transform

	//  End recursion at 2 points.
	if (k == 1) {
		__m128d a = T[0];
		__m128d b = T[1];
		T[0] = _mm_add_pd(a, b);
		T[1] = _mm_sub_pd(a, b);
		return;
	}
	else if (k == 2) {
		//  Perform FFT reduction into two halves.

		//  Grab Twiddle Factor
		__m256d r0 = _mm256_set_pd(0, 0, 1, 1);
		__m256d i0 = _mm256_set_pd(1, 1, 0, 0);

		//  Grab elements
		__m256d a0 = ((__m256d*)T)[0];
		__m256d b0 = ((__m256d*)T)[1];

		//  Perform butterfly
		__m256d c0, d0;
		c0 = _mm256_add_pd(a0, b0);
		d0 = _mm256_sub_pd(a0, b0);

		((__m256d*)T)[0] = c0;

		//  Multiply by twiddle factor.
		c0 = _mm256_mul_pd(d0, r0);
		d0 = _mm256_mul_pd(_mm256_shuffle_pd(d0, d0, 5), i0);
		c0 = _mm256_addsub_pd(c0, d0);

		((__m256d*)T)[1] = c0;


		__m256d a = _mm256_set_m128d(T[2], T[0]);
		__m256d b = _mm256_set_m128d(T[3], T[1]);
		__m256d c = _mm256_add_pd(a, b);
		__m256d d = _mm256_sub_pd(a, b);
		T[0] = _mm256_extractf128_pd(c, 0);
		T[1] = _mm256_extractf128_pd(d, 0);
		T[2] = _mm256_extractf128_pd(c, 1);
		T[3] = _mm256_extractf128_pd(d, 1);

		return;
	}

	size_t length = (size_t)1 << k;
	size_t half_length = length / 2;

	//  Get local twiddle table.
	my_complex* local_table = twiddle_table[k];

	//  Perform FFT reduction into two halves.
	for (size_t c = 0; c < half_length; c++) {
		//  Grab Twiddle Factor
		__m128d r0 = _mm_loaddup_pd(&local_table[c].r);
		__m128d i0 = _mm_loaddup_pd(&local_table[c].i);

		//  Grab elements
		__m128d a0 = T[c];
		__m128d b0 = T[c + half_length];

		//  Perform butterfly
		__m128d c0, d0;
		c0 = _mm_add_pd(a0, b0);
		d0 = _mm_sub_pd(a0, b0);

		T[c] = c0;

		//  Multiply by twiddle factor.
		c0 = _mm_mul_pd(d0, r0);
		d0 = _mm_mul_pd(_mm_shuffle_pd(d0, d0, 1), i0);
		c0 = _mm_addsub_pd(c0, d0);

		T[c + half_length] = c0;
	}

	if (threads == 1) {
		//  Recursively perform FFT on lower elements.
		fft_forward(T, k - 1, 1);

		//  Recursively perform FFT on upper elements.
		fft_forward(T + half_length, k - 1, 1);
	}
	else {
		int tds0 = threads / 2;
		int tds1 = threads - tds0;
		//  Recursively perform FFT on lower elements.
		auto f = std::async(std::launch::async, &fft_forward, T, k - 1, tds0);

		//  Recursively perform FFT on upper elements.
		fft_forward(T + half_length, k - 1, tds1);
		f.wait();
	}
}
void fft_inverse(__m128d* T, int k, int threads) {
	//  Fast Fourier Transform
	//  This function performs an inverse FFT of length 2^k.

	//  This is a Decimation-in-Time (DIT) FFT.
	//  The frequency domain input must be in bit-reversed order.

	//Parameters:
	//  -   T           -   Pointer to array.
	//  -   k           -   2^k is the size of the transform

	//  End recursion at 2 points.
	if (k == 1) {
		__m128d a = T[0];
		__m128d b = T[1];
		T[0] = _mm_add_pd(a, b);
		T[1] = _mm_sub_pd(a, b);
		return;
	}
	else if (k == 2) {

		__m256d a = _mm256_set_m128d(T[2], T[0]);
		__m256d b = _mm256_set_m128d(T[3], T[1]);
		__m256d c = _mm256_add_pd(a, b);
		__m256d d = _mm256_sub_pd(a, b);
		T[0] = _mm256_extractf128_pd(c, 0);
		T[1] = _mm256_extractf128_pd(d, 0);
		T[2] = _mm256_extractf128_pd(c, 1);
		T[3] = _mm256_extractf128_pd(d, 1);


		__m256d r0 = _mm256_set_pd(0, 0, 1, 1);
		__m128d i0_1 = _mm_xor_pd(_mm_set1_pd(0), _mm_set1_pd(-0.0));
		__m128d i0_2 = _mm_xor_pd(_mm_set1_pd(1), _mm_set1_pd(-0.0));

		__m256d i0 = _mm256_set_m128d(i0_2, i0_1);

		//  Grab elements
		__m256d a0 = ((__m256d*)T)[0];
		__m256d b0 = ((__m256d*)T)[1];

		//  Perform butterfly
		__m256d c0, d0;

		//  Multiply by twiddle factor.
		c0 = _mm256_mul_pd(b0, r0);
		d0 = _mm256_mul_pd(_mm256_shuffle_pd(b0, b0, 5), i0);
		c0 = _mm256_addsub_pd(c0, d0);

		b0 = _mm256_add_pd(a0, c0);
		d0 = _mm256_sub_pd(a0, c0);

		((__m256d*)T)[0] = b0;
		((__m256d*)T)[1] = d0;

		return;
	}

	size_t length = (size_t)1 << k;
	size_t half_length = length / 2;

	if (threads == 1) {
		//  Recursively perform FFT on lower elements.
		fft_inverse(T, k - 1, 1);

		//  Recursively perform FFT on upper elements.
		fft_inverse(T + half_length, k - 1, 1);
	}
	else {
		int tds0 = threads / 2;
		int tds1 = threads - tds0;
		//  Recursively perform FFT on lower elements.
		auto f = std::async(std::launch::async, &fft_inverse, T, k - 1, tds0);

		//  Recursively perform FFT on upper elements.
		fft_inverse(T + half_length, k - 1, tds1);

		f.wait();
	}

	//  Get local twiddle table.
	my_complex* local_table = twiddle_table[k];

	//  Perform FFT reduction into two halves.
	for (size_t c = 0; c < half_length; c++) {
		//  Grab Twiddle Factor
		__m128d r0 = _mm_loaddup_pd(&local_table[c].r);
		__m128d i0 = _mm_loaddup_pd(&local_table[c].i);
		i0 = _mm_xor_pd(i0, _mm_set1_pd(-0.0));

		//  Grab elements
		__m128d a0 = T[c];
		__m128d b0 = T[c + half_length];

		//  Perform butterfly
		__m128d c0, d0;

		//  Multiply by twiddle factor.
		c0 = _mm_mul_pd(b0, r0);
		d0 = _mm_mul_pd(_mm_shuffle_pd(b0, b0, 1), i0);
		c0 = _mm_addsub_pd(c0, d0);

		b0 = _mm_add_pd(a0, c0);
		d0 = _mm_sub_pd(a0, c0);

		T[c] = b0;
		T[c + half_length] = d0;
	}
}
void fft_pointwise(__m128d* T, __m128d* A, int k) {
	//  Performs pointwise multiplications of two FFT arrays.

	//Parameters:
	//  -   T           -   Pointer to array.
	//  -   k           -   2^k is the size of the transform

	size_t length = (size_t)1 << k;
	for (size_t c = 0; c < length; c++) {
		__m128d a0 = T[c];
		__m128d b0 = A[c];
		__m128d c0, d0;
		c0 = _mm_mul_pd(a0, _mm_unpacklo_pd(b0, b0));
		d0 = _mm_mul_pd(_mm_shuffle_pd(a0, a0, 1), _mm_unpackhi_pd(b0, b0));
		T[c] = _mm_addsub_pd(c0, d0);
	}
}
void int_to_fft(__m128d* T, int k, const uint32_t* A, size_t AL) {
	//  Convert word array into FFT array. Put 3 decimal digits per complex point.

	//Parameters:
	//  -   T   -   FFT array
	//  -   k   -   2^k is the size of the transform
	//  -   A   -   word array
	//  -   AL  -   length of word array

	size_t fft_length = (size_t)1 << k;
	__m128d* Tstop = T + fft_length;

	//  Since there are 9 digits per word and we want to put 3 digits per
	//  point, the length of the transform must be at least 3 times the word
	//  length of the input.
	if (fft_length < 4 * AL)
		throw "FFT length is too small.";

	//  Convert
	for (size_t c = 0; c < AL; c++) {
		uint32_t word = A[c];

		*T++ = _mm_set_sd(word % 256);
		word /= 256;
		*T++ = _mm_set_sd(word % 256);
		word /= 256;
		*T++ = _mm_set_sd(word % 256);
		word /= 256;
		*T++ = _mm_set_sd(word);
	}

	//  Pad the rest with zeros.
	while (T < Tstop)
		*T++ = _mm_setzero_pd();
}
void fft_to_int(__m128d* T, int k, uint32_t* A, size_t AL) {
	//  Convert FFT array back to word array. Perform rounding and carryout.

	//Parameters:
	//  -   T   -   FFT array
	//  -   A   -   word array
	//  -   AL  -   length of word array

	//  Compute Scaling Factor
	size_t fft_length = (size_t)1 << k;
	double scale = 1. / fft_length;

	//  Since there are 9 digits per word and we want to put 3 digits per
	//  point, the length of the transform must be at least 3 times the word
	//  length of the input.
	if (fft_length < 4 * AL)
		throw "FFT length is too small.";

	//  Round and carry out.
	uint64_t carry = 0;
	for (size_t c = 0; c < AL; c++) {
		double   f_point;
		uint64_t i_point;
		uint32_t word;

		f_point = ((double*)T++)[0] * scale;    //  Load and scale
		i_point = (uint64_t)(f_point + 0.5);    //  Round
		carry += i_point;                       //  Add to carry
		word = carry % 256;                    //  Get 3 digits.
		carry /= 256;

		f_point = ((double*)T++)[0] * scale;    //  Load and scale
		i_point = (uint64_t)(f_point + 0.5);    //  Round
		carry += i_point;                       //  Add to carry
		word += (carry % 256) * 256;          //  Get 3 digits.
		carry /= 256;

		f_point = ((double*)T++)[0] * scale;    //  Load and scale
		i_point = (uint64_t)(f_point + 0.5);    //  Round
		carry += i_point;                       //  Add to carry
		word += (carry % 256) * 256 * 256;          //  Get 3 digits.
		carry /= 256;

		f_point = ((double*)T++)[0] * scale;    //  Load and scale
		i_point = (uint64_t)(f_point + 0.5);    //  Round
		carry += i_point;                       //  Add to carry
		word += (carry % 256) * 256 * 256 * 256;          //  Get 3 digits.
		carry /= 256;

		A[c] = word;
	}
}