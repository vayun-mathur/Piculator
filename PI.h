#pragma once
#if defined(_MSC_VER) && (_MSC_VER <= 1600)
#define USE_CHRONO 0
#else
#define USE_CHRONO 1
#endif

#ifdef _MSC_VER
#pragma warning(disable:4996)   //  fopen() deprecation
#endif


#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <complex>
#include <algorithm>
#include <memory>
#include <iostream>
using std::cout;
using std::endl;
using std::complex;

#include "BigFloat.h"

#if USE_CHRONO
#include <chrono>
#else
#include <time.h>
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Helpers
double wall_clock() {
	//  Get the clock in seconds.
#if USE_CHRONO
	auto ratio_object = std::chrono::high_resolution_clock::period();
	double ratio = (double)ratio_object.num / ratio_object.den;
	return std::chrono::high_resolution_clock::now().time_since_epoch().count() * ratio;
#else
	return (double)clock() / CLOCKS_PER_SEC;
#endif
}
void dump_to_file(const char* path, const std::string& str) {
	//  Dump a string to a file.

	FILE* file = fopen(path, "wb");
	if (file == NULL)
		throw "Cannot Create File";

	fwrite(str.c_str(), 1, str.size(), file);
	fclose(file);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  e
double logf_approx(double x) {
	//  Returns a very good approximation to log(x!).
	//  log(x!) ~ (x + 1/2) * (log(x) - 1) + (log(2*pi) + 1) / 2
	//  This approximation gets better as x is larger.
	if (x <= 1) return 0;
	return (x + .5) * (log(x) - 1) + (1.4189385332046727417803297364056176398613974736378);
}
size_t e_terms(size_t p) {
	//  Returns the # of terms needed to reach a precision of p.

	//  The taylor series converges to log(x!) / log(10) decimal digits after
	//  x terms. So to find the number of terms needed to reach a precision of p
	//  we need to solve this question for x:
	//      p = log(x!) / log(1000000000)

	//  This function solves this equation via binary search.

	double sizeL = (double)p * 20.723265836946411156161923092159277868409913397659 + 1;

	size_t a = 0;
	size_t b = 1;

	//  Double up
	while (logf_approx((double)b) < sizeL)
		b <<= 1;

	//  Binary search
	while (b - a > 1) {
		size_t m = (a + b) >> 1;

		if (logf_approx((double)m) < sizeL)
			a = m;
		else
			b = m;
	}

	return b + 2;
}
void e_BSR(BigFloat& P, BigFloat& Q, uint32_t a, uint32_t b) {
	//  Binary Splitting recursion for exp(1).

	if (b - a == 1) {
		P = BigFloat(1);
		Q = BigFloat(b);
		return;
	}

	uint32_t m = (a + b) / 2;

	BigFloat P0, Q0, P1, Q1;
	e_BSR(P0, Q0, a, m);
	e_BSR(P1, Q1, m, b);

	P = P0.mul(Q1).add(P1);
	Q = Q0.mul(Q1);
}
void e(size_t digits) {
	//  The leading 2 doesn't count.
	digits++;

	size_t p = (digits + 8) / 9;
	size_t terms = e_terms(p);

	//  Limit Exceeded
	if ((uint32_t)terms != terms)
		throw "Limit Exceeded";

	cout << "Computing e..." << endl;
	cout << "Algorithm: Taylor Series of exp(1)" << endl << endl;

	double time0 = wall_clock();

	cout << "Summing Series... " << terms << " terms" << endl;
	BigFloat P, Q;
	e_BSR(P, Q, 0, (uint32_t)terms);
	double time1 = wall_clock();
	cout << "Time: " << time1 - time0 << endl;

	cout << "Division... " << endl;
	P = P.div(Q, p).add(BigFloat(1), p);
	double time2 = wall_clock();
	cout << "Time: " << time2 - time1 << endl;

	cout << "Total Time = " << time2 - time0 << endl << endl;

	dump_to_file("e.txt", P.to_string(digits));
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Pi
void Pi_BSR(BigFloat& P, BigFloat& Q, BigFloat& R, uint32_t a, uint32_t b, size_t p) {
	//  Binary Splitting recursion for the Chudnovsky Formula.

	if (b - a == 1) {
		//  P = (13591409 + 545140134 b)(2b-1)(6b-5)(6b-1) (-1)^b
		P = BigFloat(b).mul(545140134);
		P = P.add(BigFloat(13591409));
		P = P.mul(2 * b - 1);
		P = P.mul(6 * b - 5);
		P = P.mul(6 * b - 1);
		if (b % 2 == 1)
			P.negate();

		//  Q = 10939058860032000 * b^3
		Q = BigFloat(b);
		Q = Q.mul(Q).mul(Q).mul(26726400).mul(409297880);

		//  R = (2b-1)(6b-5)(6b-1)
		R = BigFloat(2 * b - 1);
		R = R.mul(6 * b - 5);
		R = R.mul(6 * b - 1);

		return;
	}

	uint32_t m = (a + b) / 2;

	BigFloat P0, Q0, R0, P1, Q1, R1;
	Pi_BSR(P0, Q0, R0, a, m, p);
	Pi_BSR(P1, Q1, R1, m, b, p);

	P = P0.mul(Q1, p).add(P1.mul(R0, p), p);
	Q = Q0.mul(Q1, p);
	R = R0.mul(R1, p);
}
void Pi(size_t digits) {
	//  The leading 3 doesn't count.
	digits++;

	size_t p = (digits + 8) / 9;
	size_t terms = (size_t)(p * 0.6346230241342037371474889163921741077188431452678) + 1;

	//  Limit Exceeded
	if ((uint32_t)terms != terms)
		throw "Limit Exceeded";

	cout << "Computing Pi..." << endl;
	cout << "Algorithm: Chudnovsky Formula" << endl << endl;

	double time0 = wall_clock();

	cout << "Summing Series... " << terms << " terms" << endl;
	BigFloat P, Q, R;
	Pi_BSR(P, Q, R, 0, (uint32_t)terms, p);
	P = Q.mul(13591409).add(P, p);
	Q = Q.mul(4270934400);
	double time1 = wall_clock();
	cout << "Time: " << time1 - time0 << endl;

	cout << "Division... " << endl;
	P = Q.div(P, p);
	double time2 = wall_clock();
	cout << "Time: " << time2 - time1 << endl;

	cout << "InvSqrt... " << endl;
	Q = invsqrt(10005, p);
	double time3 = wall_clock();
	cout << "Time: " << time3 - time2 << endl;

	cout << "Final Multiply... " << endl;
	P = P.mul(Q, p);
	double time4 = wall_clock();
	cout << "Time: " << time4 - time3 << endl;

	cout << "Total Time = " << time4 - time0 << endl << endl;

	dump_to_file("pi.txt", P.to_string(digits));
}