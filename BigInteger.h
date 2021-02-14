#pragma once
#include "BigNum.h"
inline index int_msb;
inline index int_lsb;
class BigInteger
	: public BigNum
{
public:
	BigInteger() : BigNum(int_msb, int_lsb) {

	}
	BigInteger(word w) : BigNum(w, int_msb, int_lsb) {

	}
	BigInteger(const BigNum& other) : BigNum(other, int_msb, int_lsb) {

	}
};

inline index float_msb;
inline index float_lsb;
class BigFloat
	: public BigNum
{
public:
	BigFloat() : BigNum(float_msb, float_lsb) {

	}
	BigFloat(word w) : BigNum(w, float_msb, float_lsb) {

	}
	BigFloat(const BigNum& other) : BigNum(other, float_msb, float_lsb) {

	}
};
