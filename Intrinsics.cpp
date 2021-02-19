#include "Intrinsics.h"
#include <intrin.h>

unsigned __int64 _shiftright128(unsigned __int64 low, unsigned __int64 high, unsigned char shift)
{
    return __shiftright128(low, high, shift);
}

unsigned __int64 _shiftleft128(unsigned __int64 low, unsigned __int64 high, unsigned char shift)
{
    return __shiftleft128(low, high, shift);
}

unsigned __int64 mulx(unsigned __int64 src1, unsigned __int64 src2, unsigned __int64* high)
{
    return _mulx_u64(src1, src2, high);
}

bool __cdecl adc(bool c_in, unsigned __int64 src1, unsigned __int64 src2, unsigned __int64* out)
{
    return _addcarryx_u64(c_in, src1, src2, out);
}

bool __cdecl subb(bool c_in, unsigned __int64 src1, unsigned __int64 src2, unsigned __int64* out)
{
    return _subborrow_u64(c_in, src1, src2, out);
}

unsigned __int64 __cdecl div(unsigned __int64 high_dividend, unsigned __int64 low_dividend, unsigned __int64 divisor, unsigned __int64* remainder)
{
    return _udiv128(high_dividend, low_dividend, divisor, remainder);
}
