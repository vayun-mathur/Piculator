#include "BigInteger.h"
#include <intrin.h>
#include <immintrin.h>
#include <exception>
#include <iomanip>
#include "NNT.h"


BigInteger::BigInteger()
	: arr(new word[msb])
{
	memset(arr, 0, sizeof(word) * msb);
}

BigInteger::BigInteger(word w)
	: BigInteger()
{
	*this = w;
}

BigInteger::BigInteger(const BigInteger& other)
	: arr(other.arr)
{
}

BigInteger BigInteger::operator+(const BigInteger& other) const
{
	BigInteger res;
	unsigned char carry = 0;
	for (index bit = 0; bit < msb; ++bit) {
		carry = _addcarry_u64(carry, this->arr[bit], other.arr[bit], res.arr + bit);
	}
	return res;
}

BigInteger BigInteger::operator-(const BigInteger& other) const
{
	BigInteger res;
	unsigned char carry = 0;
	for (index bit = 0; bit < msb; ++bit) {
		carry = _subborrow_u64(carry, this->arr[bit], other.arr[bit], res.arr + bit);
	}
	return res;
}

BigInteger& BigInteger::operator+=(const BigInteger& other)
{
	unsigned char carry = 0;
	for (index bit = 0; bit < msb; ++bit) {
		carry = _addcarry_u64(carry, this->arr[bit], other.arr[bit], this->arr + bit);
	}
	return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& other)
{
	unsigned char carry = 0;
	for (index bit = 0; bit < msb; ++bit) {
		carry = _subborrow_u64(carry, this->arr[bit], other.arr[bit], this->arr + bit);
	}
	return *this;
}

void carryTrain(word* arr, index index, uint64_t value) {
	bool carry = _addcarry_u64(0, arr[index], value, arr + index);
	while (index < msb && carry) {
		carry = _addcarry_u64(carry, arr[index], 0, arr + index);
		++index;
	}
}

BigInteger BigInteger::operator*(const BigInteger& other) const
{
	int n = 8 * msb;
	DWORD* x, * y, * xx, * yy, a;
	x = new DWORD[n], xx = new DWORD[n];
	y = new DWORD[n], yy = new DWORD[n];
	for (int i = 0; i < n; i++) {
		x[i] = ((uint8_t*)this->arr)[i];
		y[i] = ((uint8_t*)other.arr)[i];
	}
	//NTT
	NNT ntt;
	ntt.NTT(xx, x, n);
	ntt.NTT(yy, y);

	// Convolution
	for (int i = 0; i < n; i++) xx[i] = ntt.modmul(xx[i], yy[i]);

	//INTT
	ntt.iNTT(yy, xx);
	BigInteger res = 0;
	for (index i = 0; i < msb; i++) {
		word v1 = 0;
		word v2 = 0;
		for (int k = 0; k < 8; k++) {
			v2 += _addcarry_u64(0, v1, ((word)yy[i * 8 + k]) << (k * 8), &v1);
			v2 += __shiftleft128((word)yy[i * 8 + k], 0, k * 8);
		}

		carryTrain(res.arr, i, v1);
		carryTrain(res.arr, i + 1, v2);
	}


	delete[] x;
	delete[] y;
	delete[] xx;
	delete[] yy;

	return res;
}

BigInteger BigInteger::operator*(word other) const
{
	BigInteger res;
	word high = 0;
	for (index bit = 0; bit < msb; ++bit) {
		word high_temp;
		res.arr[bit] = _umul128(this->arr[bit], other, &high_temp);
		high_temp += _addcarry_u64(0, res.arr[bit], high, res.arr + bit);
	}
	return res;
}

BigInteger& BigInteger::operator*=(const BigInteger& other)
{
	BigInteger& _this = *this;
	_this = _this * other;
	return _this;
}

BigInteger& BigInteger::operator*=(word other)
{
	word high = 0;
	for (index bit = 0; bit < msb; ++bit) {
		word high_temp;
		this->arr[bit] = _umul128(this->arr[bit], other, &high_temp);
		high_temp += _addcarry_u64(0, this->arr[bit], high, this->arr + bit);
	}
	return *this;
}

BigInteger BigInteger::operator/(const BigInteger& other) const
{
	BigInteger dividend;
	BigInteger divisor;
	dividend = *this;
	divisor = other;
	BigInteger res = 0;
	index bit = 0; //TODO DIVIDE HIGHER
	while (bit >= 0) {
		if (divisor < dividend) {
			word mand = 1ull << 63;
			word shift = 63;
			word bit_val = 0;
			BigInteger mulres;
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
	return res;
}

BigInteger BigInteger::operator/(word other) const
{
	BigInteger res;
	word high = 0;
	for (index bit = msb - 1; bit >= 0; --bit) {
		res.arr[bit] = _udiv128(high, this->arr[bit], other, &high);
	}
	return res;
}

BigInteger& BigInteger::operator/=(const BigInteger& other)
{
	BigInteger& _this = *this;
	_this = _this / other;
	return _this;
}

BigInteger& BigInteger::operator/=(word other)
{
	word high = 0;
	for (index bit = msb - 1; bit >= 0; --bit) {
		this->arr[bit] = _udiv128(high, this->arr[bit], other, &high);
	}
	return *this;
}

BigInteger BigInteger::operator%(const BigInteger& other) const
{
	BigInteger dividend;
	BigInteger divisor;
	dividend = *this;
	divisor = other;
	BigInteger res;
	index bit = msb - 1;
	while (bit >= 0) {
		if (divisor < dividend) {
			word mand = 1ull << 63;
			word bit_val = 0;
			BigInteger mulres = divisor << 63ull;
			while (mand) {
				if (mulres < dividend) {
					dividend -= mulres;
					bit_val += mand;
				}
				mand >>= 1;
				mulres >>= 1;
			}
			res.arr[bit] = bit_val;
		}
		divisor >>= 64;
		bit--;
	}
	return dividend;
}

word BigInteger::operator%(word other) const
{
	word high = 0;
	for (index bit = msb - 1; bit >= 0; --bit) {
		_udiv128(high, this->arr[bit], other, &high);
	}
	return high;
}

BigInteger& BigInteger::operator++()
{
	unsigned char carry = 1;
	for (index bit = 0; bit < msb && carry; ++bit) {
		carry = _addcarry_u64(carry, this->arr[bit], 0, this->arr + bit);
	}
	return *this;
}

BigInteger BigInteger::operator++(int)
{
	BigInteger old = *this;
	operator++();
	return old;
}

BigInteger& BigInteger::operator--()
{
	unsigned char carry = 1;
	for (index bit = 0; bit < msb && carry; ++bit) {
		carry = _subborrow_u64(carry, this->arr[bit], 0, this->arr + bit);
	}
	return *this;
}

BigInteger BigInteger::operator--(int)
{
	BigInteger old = *this;
	operator--();
	return old;
}

BigInteger& BigInteger::operator=(word w)
{
	for (index bit = 1; bit < msb; ++bit) {
		this->arr[bit] = 0;
	}
	this->arr[0] = w;
	return *this;
}

BigInteger& BigInteger::operator=(const BigInteger& other)
{
	for (index bit = 0; bit < msb; ++bit) {
		this->arr[bit] = other.arr[bit];
	}

	return *this;
}

BigInteger BigInteger::operator&(const BigInteger& other) const
{
	BigInteger res;
	for (index bit = 0; bit < msb; ++bit) {
		res.arr[bit] = this->arr[bit] & other.arr[bit];
	}
	return res;
}

BigInteger BigInteger::operator|(const BigInteger& other) const
{
	BigInteger res;
	for (index bit = 0; bit < msb; ++bit) {
		res.arr[bit] = this->arr[bit] | other.arr[bit];
	}
	return res;
}

BigInteger BigInteger::operator^(const BigInteger& other) const
{
	BigInteger res;
	for (index bit = 0; bit < msb; ++bit) {
		res.arr[bit] = this->arr[bit] ^ other.arr[bit];
	}
	return res;
}

BigInteger& BigInteger::operator&=(const BigInteger& other)
{
	for (index bit = 0; bit < msb; ++bit) {
		this->arr[bit] &= other.arr[bit];
	}
	return *this;
}

BigInteger& BigInteger::operator|=(const BigInteger& other)
{
	for (index bit = 0; bit < msb; ++bit) {
		this->arr[bit] |= other.arr[bit];
	}
	return *this;
}

BigInteger& BigInteger::operator^=(const BigInteger& other)
{
	for (index bit = 0; bit < msb; ++bit) {
		this->arr[bit] ^= other.arr[bit];
	}
	return *this;
}

BigInteger BigInteger::operator~() const
{
	BigInteger res;
	for (index bit = 0; bit < msb; ++bit) {
		res.arr[bit] = ~this->arr[bit];
	}
	return res;
}

BigInteger BigInteger::operator<<(word shift_count) const
{
	if (shift_count > msb * 64) throw std::exception("Shift count too large");
	BigInteger res;
	for (index bit = 0; bit < shift_count / 64; ++bit) {
		res.arr[bit] = 0;
	}
	for (index bit = shift_count / 64; bit < msb; ++bit) {
		res.arr[bit] = this->arr[bit - shift_count/64];
	}
	shift_count %= 64;
	word low = 0;
	for (index bit = 0; bit < msb; bit++) {
		res.arr[bit] = __shiftleft128(low, this->arr[bit], shift_count);
		low = this->arr[bit];
	}
	return res;
}

BigInteger BigInteger::operator>>(word shift_count) const
{
	if (shift_count > msb * 64) throw std::exception("Shift count too large");
	BigInteger res;
	for (index bit = 0; bit < msb - shift_count / 64; ++bit) {
		res.arr[bit] = this->arr[bit + shift_count/64];
	}
	for (index bit = msb - shift_count / 64; bit < msb; ++bit) {
		res.arr[bit] = 0;
	}
	shift_count %= 64;
	word high = 0;
	for (index bit = msb - 1; bit >= 0; bit--) {
		res.arr[bit] = __shiftright128(this->arr[bit], high, shift_count);
		high = this->arr[bit];
	}
	return res;
}

BigInteger& BigInteger::operator<<=(word shift_count)
{
	if (shift_count > msb * 64) throw std::exception("Shift count too large");
	for (index bit = msb - 1; bit >= shift_count / 64; --bit) {
		this->arr[bit] = this->arr[bit - shift_count/64];
	}
	for (index bit = 0; bit < shift_count / 64; ++bit) {
		this->arr[bit] = 0;
	}
	shift_count %= 64;
	word low = 0;
	for (index bit = 0; bit < msb; bit++) {
		word curr_bit = this->arr[bit];
		this->arr[bit] = __shiftleft128(low, curr_bit, shift_count);
		low = curr_bit;
	}
	return *this;
}

BigInteger& BigInteger::operator>>=(word shift_count)
{
	if (shift_count > msb * 64) throw std::exception("Shift count too large");
	for (index bit = msb - 1 - shift_count / 64; bit >= 0; --bit) {
		this->arr[bit] = this->arr[bit + shift_count/64];
	}
	for (index bit = msb - shift_count / 64; bit < msb; ++bit) {
		this->arr[bit] = 0;
	}
	shift_count %= 64;
	word high = 0;
	for (index bit = msb - 1; bit >= 0; bit--) {
		word curr_bit = this->arr[bit];
		this->arr[bit] = __shiftright128(curr_bit, high, shift_count);
		high = curr_bit;
	}
	return *this;
}

bool BigInteger::operator&&(const BigInteger& other) const
{
	return bool(*this) && bool(other);
}

bool BigInteger::operator||(const BigInteger& other) const
{
	return bool(*this) || bool(other);
}

bool BigInteger::operator!() const
{
	return !bool(*this);
}

BigInteger::operator bool() const
{
	for (index bit = msb - 1; bit >= 0; --bit) {
		if (this->arr[bit] != 0) return true;
	}
	return false;
}

bool BigInteger::operator<(const BigInteger& other) const
{
	for (index bit = msb - 1; bit >= 0; --bit) {
		if (this->arr[bit] < other.arr[bit]) return true;
		if (this->arr[bit] > other.arr[bit]) return false;
	}
	return false;
}

bool BigInteger::operator>(const BigInteger& other) const
{
	for (index bit = msb - 1; bit >= 0; --bit) {
		if (this->arr[bit] < other.arr[bit]) return false;
		if (this->arr[bit] > other.arr[bit]) return true;
	}
	return false;
}

bool BigInteger::operator<=(const BigInteger& other) const
{
	for (index bit = msb - 1; bit >= 0; --bit) {
		if (this->arr[bit] < other.arr[bit]) return true;
		if (this->arr[bit] > other.arr[bit]) return false;
	}
	return true;
}

bool BigInteger::operator>=(const BigInteger& other) const
{
	for (index bit = msb - 1; bit >= 0; --bit) {
		if (this->arr[bit] < other.arr[bit]) return false;
		if (this->arr[bit] > other.arr[bit]) return true;
	}
	return true;
}

bool BigInteger::operator==(const BigInteger& other) const
{
	for (index bit = msb - 1; bit >= 0; --bit) {
		if (this->arr[bit] != other.arr[bit]) return false;
	}
	return true;
}

bool BigInteger::operator!=(const BigInteger& other) const
{
	for (index bit = msb - 1; bit >= 0; --bit) {
		if (this->arr[bit] == other.arr[bit]) return true;
	}
	return false;
}

std::ostream& operator<<(std::ostream& os, const BigInteger& ref)
{
	if ((os.flags() & std::ios_base::hex) != 0) { //hex
		index bit = msb - 1;
		while (ref.arr[bit] == 0 && bit != 0) bit--;
		os << "0x";
		while (bit != 0) {
			os << std::setw(16) << std::setfill('0') << ref.arr[bit] << " ";
			bit--;
		}
		os << std::setw(16) << std::setfill('0') << ref.arr[bit];
	}
	if ((os.flags() & std::ios_base::dec) != 0) { //dec

	}
	return os;
}
