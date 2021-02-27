#include "BigNum.h"
#include "Intrinsics.h"
#include <exception>
#include <iomanip>
#include <sstream>
#include "NNT.h"

//TODO: ADD NUMBERS OF DIFFERENT SIZES

struct array_deleter
{
	index lsb;
	array_deleter(index lsb) : lsb(lsb) {}
	void operator ()(word const* p)
	{
		printf("Deleting ptr %p\n", p);
		delete[](p + lsb);
	}
};

BigNum::BigNum(index msb, index lsb)
	: msb(msb), lsb(lsb), ptr(new word[msb-lsb]), arr(ptr.get() - lsb), negative(false)
{
	memset(arr + lsb, 0, sizeof(word) * (msb - lsb));
}

BigNum::BigNum(int64 w, index msb, index lsb)
	: BigNum(msb, lsb)
{
	*this = w;
}

BigNum::BigNum(const BigNum& other, index msb, index lsb)
	: BigNum(msb, lsb)
{
	*this = other;
}

BigNum BigNum::operator+(const BigNum& other) const
{
	BigNum res(msb, lsb);
	bool carry = 0;
	for (index bit = lsb; bit < msb; ++bit) {
		carry = adc(carry, this->arr[bit], other.arr[bit], res.arr + bit);
	}
	return res;
}

BigNum BigNum::operator-(const BigNum& other) const
{
	BigNum res(msb, lsb);
	bool carry = 0;
	for (index bit = lsb; bit < msb; ++bit) {
		carry = subb(carry, this->arr[bit], other.arr[bit], res.arr + bit);
	}
	return res;
}

BigNum& BigNum::operator+=(const BigNum& other)
{
	bool carry = 0;
	for (index bit = lsb; bit < msb; ++bit) {
		carry = adc(carry, this->arr[bit], other.arr[bit], this->arr + bit);
	}
	return *this;
}

BigNum& BigNum::operator-=(const BigNum& other)
{
	bool carry = 0;
	for (index bit = lsb; bit < msb; ++bit) {
		carry = subb(carry, this->arr[bit], other.arr[bit], this->arr + bit);
	}
	return *this;
}

void carryTrain(word* arr, index msb, index index, uint64_t value) {
	bool carry = adc(0, arr[index], value, arr + index);
	while (index < msb && carry) {
		carry = adc(carry, arr[index], 0, arr + index);
		++index;
	}
}

BigNum BigNum::operator*(const BigNum& other) const
{
	int n = 8 * (2 * (msb - lsb));
	DWORD* x, * y, * xx, * yy, a;
	x = new DWORD[n], xx = new DWORD[n];
	y = new DWORD[n], yy = new DWORD[n];
	for (int i = 0; i < n / 2; i++) {
		x[i] = ((uint8_t*)(this->arr + lsb))[i];
		y[i] = ((uint8_t*)(other.arr + lsb))[i];
	}
	for (int i = n / 2; i < n; i++) {
		x[i] = 0;
		y[i] = 0;
	}

	//NTT
	NNT ntt;
	ntt.NTT(xx, x, n);
	ntt.NTT(yy, y);

	// Convolution
	for (int i = 0; i < n; i++) xx[i] = ntt.modmul(xx[i], yy[i]);

	//INTT
	ntt.iNTT(yy, xx);

	BigNum res(msb, lsb);
	res = 0;
	for (index i = 0; i < 2 * (msb - lsb); i++) {
		word v1 = 0;
		word v2 = 0;
		for (int k = 0; k < 8; k++) {
			v2 += adc(0, v1, ((word)yy[i * 8 + k]) << (k * 8), &v1);
			v2 += _shiftleft128((word)yy[i * 8 + k], 0, k * 8);
		}
		if (i + 2 * lsb >= lsb && i + 2 * lsb < msb)
			carryTrain(res.arr, msb, i + 2 * lsb, v1);
		if (i + 2 * lsb + 1 >= lsb && i + 2 * lsb + 1 < msb)
			carryTrain(res.arr, msb, i + 2 * lsb + 1, v2);
	}
	res.negative = this->negative ^ other.negative;

	delete[] x;
	delete[] y;
	delete[] xx;
	delete[] yy;

	return res;
}

BigNum BigNum::operator*(int64 other) const
{
	BigNum res = BigNum(*this, msb, lsb);
	res *= other;
	return res;
}

BigNum& BigNum::operator*=(const BigNum& other)
{
	BigNum& _this = *this;
	_this = _this * other;
	return _this;
}

BigNum& BigNum::operator*=(int64 other)
{
	if (other < 0) {
		this->negative = !this->negative;
		other = -other;
	}
	word high = 0;
	for (index bit = lsb; bit < msb; ++bit) {
		word high_temp = 0;
		this->arr[bit] = mulx(this->arr[bit], other, &high_temp);
		high_temp += adc(0, this->arr[bit], high, this->arr + bit);
		high = high_temp;
	}
	return *this;
}
BigNum BigNum::operator/(const BigNum& other) const
{
	BigNum dividend(msb, lsb);
	dividend = *this;
	BigNum divisor(msb, lsb);
	divisor = other;
	BigNum res(msb, lsb);
	res = 0;
	index bit = 0;
	while (bit + 1 < msb && divisor < dividend) {
		divisor <<= 64;
		bit++;
	}
	while (bit >= lsb) {
		if (divisor < dividend) {
			word mand = 1ull << 63;
			word shift = 63;
			word bit_val = 0;
			BigNum mulres(msb, lsb);
			while (mand) {
				mulres = divisor << shift;
				if (mulres <= dividend) {
					dividend -= mulres;
					bit_val += mand;
				}
				mand >>= 1;
				shift--;
			}
			res.arr[bit] = bit_val;
		}
		divisor >>= 64;
		bit--;
	}
	return res;
}
BigNum BigNum::operator/(int64 other) const
{
	BigNum res = *this;
	res /= other;
	return res;
}

BigNum& BigNum::operator/=(const BigNum& other)
{
	BigNum& _this = *this;
	_this = _this / other;
	return _this;
}

BigNum& BigNum::operator/=(int64 other)
{
	if (other < 0) {
		this->negative = !this->negative;
		other = -other;
	}
	word high = 0;
	for (index bit = msb - 1; bit >= lsb; --bit) {
		this->arr[bit] = div(high, this->arr[bit], other, &high);
	}
	return *this;
}

BigNum BigNum::operator%(const BigNum& other) const
{
	BigNum dividend = *this;
	BigNum divisor = other;
	BigNum res(msb, lsb);
	res = 0;
	index bit = 0;
	while (bit + 1 < msb && divisor < dividend) {
		divisor <<= 64;
		bit++;
	}
	while (bit >= lsb) {
		if (divisor < dividend) {
			word mand = 1ull << 63;
			word shift = 63;
			word bit_val = 0;
			BigNum mulres(msb, lsb);
			while (mand) {
				mulres = divisor << shift;
				if (mulres < dividend) {
					dividend -= mulres;
					bit_val += mand;
				}
				mand >>= 1;
				shift--;
			}
			res.arr[bit] = bit_val;
		}
		divisor >>= 64;
		bit--;
	}
	return dividend;
}

word BigNum::operator%(word other) const
{
	word high = 0;
	for (index bit = msb - 1; bit >= lsb; --bit) {
		div(high, this->arr[bit], other, &high);
	}
	return high;
}

BigNum& BigNum::operator++()
{
	bool carry = 1;
	for (index bit = 0; bit < msb && carry; ++bit) {
		carry = adc(carry, this->arr[bit], 0, this->arr + bit);
	}
	return *this;
}

BigNum BigNum::operator++(int)
{
	BigNum old = *this;
	operator++();
	return old;
}

BigNum& BigNum::operator--()
{
	bool carry = 1;
	for (index bit = 0; bit < msb && carry; ++bit) {
		carry = subb(carry, this->arr[bit], 0, this->arr + bit);
	}
	return *this;
}

BigNum BigNum::operator--(int)
{
	BigNum old = *this;
	operator--();
	return old;
}

BigNum& BigNum::operator=(int64 w)
{
	for (index bit = lsb; bit < msb; ++bit) {
		this->arr[bit] = 0;
	}
	this->arr[0] = abs(w);
	this->negative = w < 0;
	return *this;
}

BigNum& BigNum::operator=(const BigNum& other)
{
	for (index bit = lsb; bit < other.lsb; bit++) this->arr[bit] = 0;
	for (index bit = std::max(other.lsb, lsb); bit < std::min(other.msb, msb); ++bit) {
		this->arr[bit] = other.arr[bit];
	}
	for (index bit = other.msb; bit < msb; bit++) this->arr[bit] = 0;

	return *this;
}

BigNum BigNum::operator&(const BigNum& other) const
{
	BigNum res(msb, lsb);
	for (index bit = lsb; bit < msb; ++bit) {
		res.arr[bit] = this->arr[bit] & other.arr[bit];
	}
	return res;
}

BigNum BigNum::operator|(const BigNum& other) const
{
	BigNum res(msb, lsb);
	for (index bit = lsb; bit < msb; ++bit) {
		res.arr[bit] = this->arr[bit] | other.arr[bit];
	}
	return res;
}

BigNum BigNum::operator^(const BigNum& other) const
{
	BigNum res(msb, lsb);
	for (index bit = lsb; bit < msb; ++bit) {
		res.arr[bit] = this->arr[bit] ^ other.arr[bit];
	}
	return res;
}

BigNum& BigNum::operator&=(const BigNum& other)
{
	for (index bit = lsb; bit < msb; ++bit) {
		this->arr[bit] &= other.arr[bit];
	}
	return *this;
}

BigNum& BigNum::operator|=(const BigNum& other)
{
	for (index bit = lsb; bit < msb; ++bit) {
		this->arr[bit] |= other.arr[bit];
	}
	return *this;
}

BigNum& BigNum::operator^=(const BigNum& other)
{
	for (index bit = lsb; bit < msb; ++bit) {
		this->arr[bit] ^= other.arr[bit];
	}
	return *this;
}

BigNum BigNum::operator~() const
{
	BigNum res(msb, lsb);
	for (index bit = lsb; bit < msb; ++bit) {
		res.arr[bit] = ~this->arr[bit];
	}
	return res;
}

BigNum BigNum::operator<<(word shift_count) const
{
	if (shift_count > (msb - lsb) * 64) throw std::exception("Shift count too large");
	BigNum res(msb, lsb);
	for (index bit = lsb; bit < lsb + shift_count / 64; ++bit) {
		res.arr[bit] = 0;
	}
	for (index bit = lsb + shift_count / 64; bit < msb; ++bit) {
		res.arr[bit] = this->arr[bit - shift_count / 64];
	}
	shift_count %= 64;
	word low = 0;
	for (index bit = lsb; bit < msb; bit++) {
		word curr_bit = res.arr[bit];
		res.arr[bit] = _shiftleft128(low, curr_bit, shift_count);
		low = curr_bit;
	}
	return res;
}

BigNum BigNum::operator>>(word shift_count) const
{
	if (shift_count > (msb - lsb) * 64) throw std::exception("Shift count too large");
	BigNum res(msb, lsb);
	for (index bit = lsb; bit < msb - shift_count / 64; ++bit) {
		res.arr[bit] = this->arr[bit + shift_count / 64];
	}
	for (index bit = msb - shift_count / 64; bit < msb; ++bit) {
		res.arr[bit] = 0;
	}
	shift_count %= 64;
	word high = 0;
	for (index bit = msb - 1; bit >= lsb; bit--) {
		word curr_bit = res.arr[bit];
		res.arr[bit] = _shiftright128(curr_bit, high, shift_count);
		high = curr_bit;
	}
	return res;
}

BigNum& BigNum::operator<<=(word shift_count)
{
	if (shift_count > (msb - lsb) * 64) throw std::exception("Shift count too large");
	for (index bit = msb - 1; bit >= lsb + (index)(shift_count / 64); --bit) {
		this->arr[bit] = this->arr[bit - shift_count / 64];
	}
	for (index bit = lsb; bit < lsb + (index)(shift_count / 64); ++bit) {
		this->arr[bit] = 0;
	}
	shift_count %= 64;
	word low = 0;
	for (index bit = lsb; bit < msb; bit++) {
		word curr_bit = this->arr[bit];
		this->arr[bit] = _shiftleft128(low, curr_bit, shift_count);
		low = curr_bit;
	}
	return *this;
}

BigNum& BigNum::operator>>=(word shift_count)
{
	if (shift_count > (msb - lsb) * 64) throw std::exception("Shift count too large");
	for (index bit = lsb; bit < msb - (index)(shift_count / 64); ++bit) {
		this->arr[bit] = this->arr[bit + (index)(shift_count / 64)];
	}
	for (index bit = msb - (index)(shift_count / 64); bit < msb; ++bit) {
		this->arr[bit] = 0;
	}
	shift_count %= 64;
	word high = 0;
	for (index bit = msb - 1; bit >= lsb; bit--) {
		word curr_bit = this->arr[bit];
		this->arr[bit] = _shiftright128(curr_bit, high, shift_count);
		high = curr_bit;
	}
	return *this;
}

BigNum::operator bool() const
{
	for (index bit = msb - 1; bit >= lsb; --bit) {
		if (this->arr[bit] != 0) return true;
	}
	return false;
}

std::string BigNum::toHexString()
{
	std::stringstream ss;
	ss << std::hex << *this;
	return ss.str();
}

int compare(const BigNum& r1, const BigNum& r2) {
	if (r1.negative && !r2.negative) return -1;
	if (!r1.negative && r2.negative) return 1;
	if (r1.negative && r2.negative) {
		for (index bit = r1.msb - 1; bit >= r1.lsb; --bit) {
			if (r1.arr[bit] < r2.arr[bit]) return 1;
			if (r1.arr[bit] > r2.arr[bit]) return -1;
		}
		return 0;
	}
	else {
		for (index bit = r1.msb - 1; bit >= r1.lsb; --bit) {
			if (r1.arr[bit] < r2.arr[bit]) return -1;
			if (r1.arr[bit] > r2.arr[bit]) return 1;
		}
		return 0;
	}
}

std::ostream& operator<<(std::ostream& os, const BigNum& ref)
{
	if ((os.flags() & std::ios_base::hex) != 0) { //hex
		index low = ref.lsb;
		while (ref.arr[low] == 0 && low != 0) low++;
		index bit = ref.msb - 1;
		while (ref.arr[bit] == 0 && bit != 0) bit--;
		os << "0x";
		os << ref.arr[bit];
		while (bit != low) {
			os << (bit == 0 ? "." : " ");
			bit--;
			os << std::setw(16) << std::setfill('0') << ref.arr[bit];
		}
	}
	if ((os.flags() & std::ios_base::dec) != 0) { //dec

	}
	return os;
}

index get_msb(word* arr, index msb, index lsb) {
	for (index bit = msb - 1; bit >= lsb; bit--) {
		if (arr[bit] != 0) {
			return bit;
		}
	}
	return lsb - 1;
}

BigNum invsqrt(const BigNum& x)
{
	BigNum r = BigNum(0ull, x.msb, x.lsb);
	r.arr[-1] = 1;
	BigNum one = BigNum(1ull, x.msb, x.lsb);
	BigNum three = BigNum(3ull, x.msb, x.lsb);

	BigNum rPrev = BigNum(0ull, x.msb, x.lsb);

	while (1) {
		if (r == rPrev) {
			break;
		}
		BigNum r2 = r * r;
		BigNum r2x = r2 * x;
		int prec = get_msb((((three - r2x) / 2ll) - one).arr, x.msb, x.lsb);
		if (prec <= x.lsb/2) break;
		rPrev = r;
		r *= ((three - r2x) / 2ll);
	}

	return r;
}