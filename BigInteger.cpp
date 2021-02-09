#include "BigInteger.h"
#include <intrin.h>
#include <immintrin.h>
#include <exception>

BigInteger::BigInteger()
	: arr(new word[msb])
{
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
		res.arr[bit] = this->arr[bit - shift_count];
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
		res.arr[bit] = this->arr[bit + shift_count];
	}
	for (index bit = msb - shift_count / 64; bit < msb; ++bit) {
		res.arr[bit] = 0;
	}
	shift_count %= 64;
	word high = 0;
	for (index bit = msb-1; bit >= 0; bit--) {
		res.arr[bit] = __shiftright128(this->arr[bit], high, shift_count);
		high = this->arr[bit];
	}
	return res;
}

BigInteger& BigInteger::operator<<=(word shift_count)
{
	if (shift_count > msb * 64) throw std::exception("Shift count too large");
	for (index bit = msb - 1; bit >= shift_count / 64; --bit) {
		this->arr[bit] = this->arr[bit - shift_count];
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
	for (index bit = msb - 1 - shift_count / 64; bit >=0; --bit) {
		this->arr[bit] = this->arr[bit + shift_count];
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
