#include "BigFloat.h"

#include <complex>
#include "FFT.h"
#include <algorithm>
#include "NTT.h"
#include <intrin.h>
#include <iomanip>
#undef min
#undef max

using std::complex;

BigFloat::BigFloat(BigFloat&& x)
	: sign(x.sign)
	, exp(x.exp)
	, L(x.L)
	, T(std::move(x.T))
{
	x.sign = true;
	x.exp = 0;
	x.L = 0;
}
BigFloat::BigFloat(const BigFloat& x)
	: sign(x.sign)
	, exp(x.exp)
	, L(x.L)
{
	T = std::unique_ptr<uint32_t[]>(new uint32_t[L]);
	for (int i = 0; i < L; i++) {
		T[i] = x.T[i];
	}
}
BigFloat& BigFloat::operator=(BigFloat&& x) {
	sign = x.sign;
	exp = x.exp;
	L = x.L;
	T = std::move(x.T);

	x.sign = true;
	x.exp = 0;
	x.L = 0;

	return *this;
}
BigFloat& BigFloat::operator=(const BigFloat& x) {
	sign = x.sign;
	exp = x.exp;
	L = x.L;
	T = std::unique_ptr<uint32_t[]>(new uint32_t[L]);
	for (int i = 0; i < L; i++) {
		T[i] = x.T[i];
	}

	return *this;
}
////////////////////////////////////////////////////////////////////////////////
//  Constructors
BigFloat::BigFloat()
	: sign(true)
	, exp(0)
	, L(0)
{}
BigFloat::BigFloat(uint32_t x, bool sign_)
	: sign(true)
	, exp(0)
	, L(1)
{
	//  Construct a BigFloat with a value of x and the specified sign.

	if (x == 0) {
		L = 0;
		return;
	}
	sign = sign_;

	T = std::unique_ptr<uint32_t[]>(new uint32_t[1]);
	T[0] = x;
}
////////////////////////////////////////////////////////////////////////////////
//  String Conversion

std::string BigFloat::to_string_dec(size_t digits) const
{
	BigFloat temp = *this;
	std::stringstream s;
	s << temp.T[temp.L - 1] << '.';
	for (int i = 0; i < (digits + 8) / 9; i++) {
		temp.T[temp.L - 1] = 0;
		temp = temp.mul(1'000'000'000);
		s << std::setw(9) << std::setfill('0') << temp.T[temp.L - 1];
	}
	return s.str().substr(0, 2 + digits);
}
std::string BigFloat::to_string_hex(size_t digits) const
{
	BigFloat temp = *this;
	std::stringstream s;
	size_t loc = temp.L - 1;
	s << temp.T[loc] << '.';
	for (int i = 0; i < (digits + 7) / 8; i++) {
		loc--;
		s << std::setw(8) << std::setfill('0') << std::hex << temp.T[loc];
	}
	return s.str().substr(0, 2 + digits);
}
////////////////////////////////////////////////////////////////////////////////
//  Getters
uint32_t BigFloat::word_at(int64_t mag) const {
	//  Returns the word at the mag'th digit place.
	//  This is useful for additions where you need to access a specific "digit place"
	//  of the operand without having to worry if it's out-of-bounds.

	//  This function is mathematically equal to:
	//      (return value) = floor(this * (10^9)^-mag) % 10^9

	if (mag < exp)
		return 0;
	if (mag >= exp + (int64_t)L)
		return 0;
	return T[(size_t)(mag - exp)];
}
int BigFloat::ucmp(const BigFloat& x) const {
	//  Compare function that ignores the sign.
	//  This is needed to determine which direction subtractions will go.

	//  Magnitude
	int64_t magA = exp + L;
	int64_t magB = x.exp + x.L;
	if (magA > magB)
		return 1;
	if (magA < magB)
		return -1;

	//  Compare
	int64_t mag = magA;
	while (mag >= exp || mag >= x.exp) {
		uint32_t wordA = word_at(mag);
		uint32_t wordB = x.word_at(mag);
		if (wordA < wordB)
			return -1;
		if (wordA > wordB)
			return 1;
		mag--;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
//  Arithmetic
void BigFloat::negate() {
	//  Negate this number.
	if (L == 0)
		return;
	sign = !sign;
}
BigFloat BigFloat::mul(uint32_t x) const {
	//  Multiply by a 32-bit unsigned integer.

	if (L == 0 || x == 0)
		return BigFloat();

	//  Compute basic fields.
	BigFloat z;
	z.sign = sign;
	z.exp = exp;
	z.L = L;

	//  Allocate mantissa
	z.T = std::unique_ptr<uint32_t[]>(new uint32_t[z.L + 1]);

	uint64_t carry = 0;
	for (size_t c = 0; c < L; c++) {
		carry += (uint64_t)T[c] * x;                //  Multiply and add to carry
		z.T[c] = (uint32_t)(carry % 4294967296);    //  Store bottom 9 digits
		carry >>= 32;                        //  Shift down the carry
	}

	//  Carry out
	if (carry != 0)
		z.T[z.L++] = (uint32_t)carry;

	return z;
}
BigFloat BigFloat::uadd(const BigFloat& x, size_t p) const {
	//  Perform addition ignoring the sign of the two operands.

	//  Magnitude
	int64_t magA = exp + L;
	int64_t magB = x.exp + x.L;
	int64_t top = std::max(magA, magB);
	int64_t bot = std::min(exp, x.exp);

	//  Target length
	int64_t TL = top - bot;

	if (p == 0) {
		//  Default value. No trunction.
		p = (size_t)TL;
	}
	else {
		//  Increase precision
		p += YCL_BIGFLOAT_EXTRA_PRECISION;
	}

	//  Perform precision truncation.
	if (TL > (int64_t)p) {
		bot = top - p;
		TL = p;
	}

	//  Compute basic fields.
	BigFloat z;
	z.sign = sign;
	z.exp = bot;
	z.L = (uint32_t)TL;

	//  Allocate mantissa
	z.T = std::unique_ptr<uint32_t[]>(new uint32_t[z.L + 1]);

	//  Add
	uint32_t carry = 0;
	for (size_t c = 0; bot < top; bot++, c++) {
		carry = _addcarry_u32(carry, word_at(bot), x.word_at(bot), &z.T[c]);
	}

	//  Carry out
	if (carry != 0) {
		z.T[z.L++] = 1;
	}

	return z;
}
BigFloat BigFloat::usub(const BigFloat& x, size_t p) const {
	//  Perform subtraction ignoring the sign of the two operands.

	//  "this" must be greater than or equal to x. Otherwise, the behavior
	//  is undefined.

	//  Magnitude
	int64_t magA = exp + L;
	int64_t magB = x.exp + x.L;
	int64_t top = std::max(magA, magB);
	int64_t bot = std::min(exp, x.exp);

	//  Truncate precision
	int64_t TL = top - bot;

	if (p == 0) {
		//  Default value. No trunction.
		p = (size_t)TL;
	}
	else {
		//  Increase precision
		p += YCL_BIGFLOAT_EXTRA_PRECISION;
	}

	if (TL > (int64_t)p) {
		bot = top - p;
		TL = p;
	}

	//  Compute basic fields.
	BigFloat z;
	z.sign = sign;
	z.exp = bot;
	z.L = (uint32_t)TL;

	//  Allocate mantissa
	z.T = std::unique_ptr<uint32_t[]>(new uint32_t[z.L]);

	//  Subtract
	int32_t carry = 0;
	for (size_t c = 0; bot < top; bot++, c++) {
		carry = _subborrow_u32(carry, word_at(bot), x.word_at(bot), &z.T[c]);
	}

	//  Strip leading zeros
	while (z.L > 0 && z.T[z.L - 1] == 0)
		z.L--;
	if (z.L == 0) {
		z.exp = 0;
		z.sign = true;
		z.T.reset();
	}

	return z;
}
BigFloat BigFloat::add(const BigFloat& x, size_t p) const {
	//  Addition

	//  The target precision is p.
	//  If (p = 0), then no truncation is done. The entire operation is done
	//  at maximum precision with no data loss.

	//  Same sign. Add.
	if (sign == x.sign)
		return uadd(x, p);

	//  this > x
	if (ucmp(x) > 0)
		return usub(x, p);

	//  this < x
	return x.usub(*this, p);
}
BigFloat BigFloat::sub(const BigFloat& x, size_t p) const {
	//  Subtraction

	//  The target precision is p.
	//  If (p = 0), then no truncation is done. The entire operation is done
	//  at maximum precision with no data loss.

	//  Different sign. Add.
	if (sign != x.sign)
		return uadd(x, p);

	//  this > x
	if (ucmp(x) > 0)
		return usub(x, p);

	//  this < x
	BigFloat z = x.usub(*this, p);
	z.negate();
	return z;
}
BigFloat BigFloat::mul(const BigFloat& x, size_t p, int threads) const {
	//  Multiplication

	//  The target precision is p.
	//  If (p = 0), then no truncation is done. The entire operation is done
	//  at maximum precision with no data loss.

	//  Either operand is zero.
	if (L == 0 || x.L == 0)
		return BigFloat();

	if (p == 0) {
		//  Default value. No trunction.
		p = L + x.L;
	}
	else {
		//  Increase precision
		p += YCL_BIGFLOAT_EXTRA_PRECISION;
	}

	//  Collect operands.
	int64_t Aexp = exp;
	int64_t Bexp = x.exp;
	size_t AL = L;
	size_t BL = x.L;
	uint32_t* AT = T.get();
	uint32_t* BT = x.T.get();

	//  Perform precision truncation.
	if (AL > p) {
		size_t chop = AL - p;
		AL = p;
		Aexp += chop;
		AT += chop;
	}
	if (BL > p) {
		size_t chop = BL - p;
		BL = p;
		Bexp += chop;
		BT += chop;
	}

	//  Compute basic fields.
	BigFloat z;
	z.sign = sign == x.sign;    //  Sign is positive if signs are equal.
	z.exp = Aexp + Bexp;       //  Add the exponents.
	z.L = AL + BL;           //  Add the lenghts for now. May need to correct later.

	//  Allocate mantissa
	z.T = std::unique_ptr<uint32_t[]>(new uint32_t[z.L]);

	//  Perform multiplication.

	//  Determine minimum FFT size.
	int k = 0;
	size_t length = 1;
	while (length < 2 * z.L) {
		length <<= 1;
		k++;
	}
	if (k < 27) {
		//  Perform a convolution using FFT.
		//  Yeah, this is slow for small sizes, but it's asympotically optimal.

		//  3 digits per point is small enough to not encounter round-off error
		//  until a transform size of 2^30.
		//  A transform length of 2^29 allows for the maximum product size to be
		//  2^29 * 3 = 1,610,612,736 decimal digits.

		//  Allocate FFT arrays
		auto Ta = (__m128d*)_mm_malloc(length * sizeof(__m128d), 16);
		auto Tb = (__m128d*)_mm_malloc(length * sizeof(__m128d), 16);

		//  Make sure the twiddle table is big enough.
		fft_ensure_table(k);

		int_to_fft(Ta, k, AT, AL);              //  Convert 1st operand
		int_to_fft(Tb, k, BT, BL);              //  Convert 2nd operand
		fft_forward(Ta, k, k>15?threads:1);                     //  Transform 1st operand
		fft_forward(Tb, k, k > 15 ? threads : 1);                     //  Transform 2nd operand
		fft_pointwise(Ta, Tb, k);               //  Pointwise multiply
		fft_inverse(Ta, k, k > 15 ? threads : 1);                     //  Perform inverse transform.
		fft_to_int(Ta, k, z.T.get(), z.L);      //  Convert back to word array.

		//  Check top word and correct length.
		if (z.T[z.L - 1] == 0)
			z.L--;

		_mm_free(Ta);
		_mm_free(Tb);
	}
	else {
		//  Perform a convolution using NTT.

		//  Allocate NTT arrays
		auto Ta = (NNT_WORD*)malloc(length * sizeof(NNT_WORD));
		auto Tb = (NNT_WORD*)malloc(length * sizeof(NNT_WORD));

		//  Make sure the twiddle table is big enough.
		ntt_ensure_table(k);

		int_to_ntt(Ta, k, AT, AL);              //  Convert 1st operand
		int_to_ntt(Tb, k, BT, BL);              //  Convert 2nd operand
		ntt_forward(Ta, k, threads);                     //  Transform 1st operand
		ntt_forward(Tb, k, threads);                     //  Transform 2nd operand
		ntt_pointwise(Ta, Tb, k);               //  Pointwise multiply
		ntt_inverse(Ta, k, threads);                     //  Perform inverse transform.
		ntt_to_int(Ta, k, z.T.get(), z.L);      //  Convert back to word array.

		//  Check top word and correct length.
		if (z.T[z.L - 1] == 0)
			z.L--;

		free(Ta);
		free(Tb);
	}

	return z;
}
BigFloat BigFloat::rcp(size_t p) const {
	//  Compute reciprocal using Newton's Method.

	//  r1 = r0 - (r0 * x - 1) * r0

	if (L == 0)
		throw "Divide by Zero";

	//  Collect operand
	int64_t Aexp = exp;
	size_t AL = L;
	uint32_t* AT = T.get();

	//  End of recursion. Generate starting point.
	if (p == 0) {
		//  Truncate precision to 3.
		p = 3;
		if (AL > p) {
			size_t chop = AL - p;
			AL = p;
			Aexp += chop;
			AT += chop;
		}

		//  Convert number to floating-point.
		double val = AT[0];
		if (AL >= 2)
			val += AT[1] * 4294967296.;
		if (AL >= 3)
			val += AT[2] * 4294967296. * 4294967296.;

		//  Compute reciprocal.
		val = 1. / val;
		Aexp = -Aexp;

		//  Scale
		while (val < 4294967296.) {
			val *= 4294967296.;
			Aexp--;
		}

		//  Rebuild a BigFloat.
		uint64_t val64 = (uint64_t)val;

		BigFloat out;
		out.sign = sign;

		out.T = std::unique_ptr<uint32_t[]>(new uint32_t[2]);
		out.T[0] = (uint32_t)(val64 % 4294967296);
		out.T[1] = (uint32_t)(val64 / 4294967296);
		out.L = 2;
		out.exp = Aexp;

		return out;
	}

	//  Half the precision
	size_t s = p / 2 + 1;
	if (p == 1) s = 0;
	if (p == 2) s = 1;

	//  Recurse at half the precision
	BigFloat T = rcp(s);

	//  r1 = r0 - (r0 * x - 1) * r0
	return T.sub(this->mul(T, p).sub(BigFloat(1), p).mul(T, p), p);
}
BigFloat BigFloat::div(const BigFloat& x, size_t p) const {
	//  Division
	return this->mul(x.rcp(p), p);
}
BigFloat invsqrt(uint32_t x, size_t p) {
	//  Compute inverse square root using Newton's Method.

	//            (  r0^2 * x - 1  )
	//  r1 = r0 - (----------------) * r0
	//            (       2        )

	if (x == 0)
		throw "Divide by Zero";

	//  End of recursion. Generate starting point.
	if (p == 0) {
		double val = 1. / sqrt((double)x);

		int64_t exponent = 0;

		//  Scale
		while (val < 4294967296.) {
			val *= 4294967296.;
			exponent--;
		}

		//  Rebuild a BigFloat.
		uint64_t val64 = (uint64_t)val;

		BigFloat out;
		out.sign = true;

		out.T = std::unique_ptr<uint32_t[]>(new uint32_t[2]);
		out.T[0] = (uint32_t)(val64 % 4294967296);
		out.T[1] = (uint32_t)(val64 / 4294967296);
		out.L = 2;
		out.exp = exponent;

		return out;
	}

	//  Half the precision
	size_t s = p / 2 + 1;
	if (p == 1) s = 0;
	if (p == 2) s = 1;

	//  Recurse at half the precision
	BigFloat T = invsqrt(x, s);

	BigFloat temp = T.mul(T, p);         //  r0^2
	temp = temp.mul(x);                 //  r0^2 * x
	temp = temp.sub(BigFloat(1), p);     //  r0^2 * x - 1
	temp = temp.mul(2147483648);         //  (r0^2 * x - 1) / 2
	temp.exp--;
	temp = temp.mul(T, p);               //  (r0^2 * x - 1) / 2 * r0
	return T.sub(temp, p);               //  r0 - (r0^2 * x - 1) / 2 * r0
}