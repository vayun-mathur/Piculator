#pragma once
#include <stdint.h>

unsigned __int64 _shiftright128(unsigned __int64 low, unsigned __int64 high, unsigned char shift);
unsigned __int64 _shiftleft128(unsigned __int64 low, unsigned __int64 high, unsigned char shift);
unsigned __int64 mulx(unsigned __int64 src1, unsigned __int64 src2, unsigned __int64* high);
bool adc(bool c_in, unsigned __int64 src1, unsigned __int64 src2, unsigned __int64* out);
bool subb(bool c_in, unsigned __int64 src1, unsigned __int64 src2, unsigned __int64* out);
unsigned __int64 div(unsigned __int64 high_dividend, unsigned __int64 low_dividend, unsigned __int64 divisor, unsigned __int64* remainder);