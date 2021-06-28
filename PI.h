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
//  Pi

size_t iterations = 0;
size_t steps;

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

    iterations++;
    printf("\r%llu steps out of %llu, %.2f%% complete", iterations, steps, ((double)iterations)/steps*100);
}

void Pi(size_t digits) {
    //  The leading 3 doesn't count.
    digits++;

    size_t p = (digits + 8) / 9;
    size_t terms = (size_t)(p * 0.6346230241342037371474889163921741077188431452678) + 1;
    steps = terms;

    //  Limit Exceeded
    if ((uint32_t)terms != terms)
        throw "Limit Exceeded";

    cout << "Computing Pi..." << endl;
    cout << "Algorithm: Chudnovsky Formula" << endl << endl;

    double time0 = wall_clock();

    cout << "Summing Series... " << terms << " terms" << endl;
    BigFloat P, Q, R;
    Pi_BSR(P, Q, R, 0, (uint32_t)terms, p);
    printf("\r"); 
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