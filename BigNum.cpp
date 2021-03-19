#include "BigNum.h"
#include "Intrinsics.h"
#include <exception>
#include <iomanip>
#include <sstream>
#include "NNT.h"

BigNum::BigNum(index msb, index lsb)
	: msb(msb), lsb(lsb), ptr(new word[msb - lsb]), arr(ptr.get() - lsb), negative(false)
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

BigNum add(const BigNum& n1, const BigNum& n2, bool neg) {
	index msb = n1.msb;
	index lsb = n1.lsb;
	BigNum res(msb, lsb);
	bool carry = 0;
	for (index bit = lsb; bit < msb; ++bit) {
		carry = adc(carry, n1.arr[bit], n2.arr[bit], res.arr + bit);
	}
	res.negative = neg;
	return res;
}

BigNum sub(const BigNum& n1, const BigNum& n2, bool neg) {
	index msb = n1.msb;
	index lsb = n1.lsb;
	BigNum res(msb, lsb);
	bool carry = 0;
	for (index bit = lsb; bit < msb; ++bit) {
		carry = subb(carry, n1.arr[bit], n2.arr[bit], res.arr + bit);
	}
	res.negative = neg;
	return res;
}

BigNum BigNum::operator+(const BigNum& other) const
{
	int cmp = compare(*this, other);
	if(negative == other.negative) return add(*this, other, negative);
	if (negative && !other.negative) {
		if (cmp <= 0) return sub(other, *this, false);
		if (cmp > 0) return sub(*this, other, true);
	}
	if (!negative && other.negative) {
		if (cmp < 0) return sub(other, *this, true);
		if (cmp >= 0) return sub(*this, other, false);
	}
}

BigNum BigNum::operator-(const BigNum& other) const
{
	BigNum addend(msb, lsb);
	addend = other;
	addend.negative = !addend.negative;
	return *this + addend;
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
	index n = 8 * (2 * (msb - lsb));
	DWORD* x, * y, * xx, * yy;
	x = new DWORD[n], xx = new DWORD[n];
	y = new DWORD[n], yy = new DWORD[n];
	for (int i = 0; i < n / 2; i++) {
		x[i] = ((uint8_t*)(this->arr + lsb))[i];
		y[i] = ((uint8_t*)(other.arr + lsb))[i];
	}
	for (index i = n / 2; i < n; i++) {
		x[i] = 0;
		y[i] = 0;
	}

	//NTT
	NNT ntt;
	ntt.NTT(xx, x, DWORD(n));
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
	BigNum res(msb, lsb);
	if (other < 0) {
		res.negative = !this->negative;
		other = -other;
	}
	else res.negative = this->negative;
	word high = 0;
	for (index bit = lsb; bit < msb; ++bit) {
		word high_temp = 0;
		res.arr[bit] = mulx(this->arr[bit], other, &high_temp);
		high_temp += adc(0, res.arr[bit], high, res.arr + bit);
		high = high_temp;
	}
	return res;
}

index get_msb(word* arr, index msb, index lsb) {
	for (index bit = msb - 1; bit >= lsb; bit--) {
		if (arr[bit] != 0) {
			return bit;
		}
	}
	return lsb - 1;
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
	BigNum res(msb, lsb);
	if (other < 0) {
		res.negative = !this->negative;
		other = -other;
	}
	else res.negative = this->negative;
	word high = 0;
	for (index bit = msb - 1; bit >= lsb; --bit) {
		res.arr[bit] = div(high, this->arr[bit], other, &high);
	}
	return res;
}

BigNum BigNum::operator%(const BigNum& other) const
{
	return *this - *this / other;
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
	negative = other.negative;

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
	index index_shift = shift_count / 64;
	if (index_shift > msb - lsb) throw std::exception("Shift count too large");
	BigNum res(msb, lsb);
	for (index bit = lsb; bit < lsb + index_shift; ++bit) {
		res.arr[bit] = 0;
	}
	for (index bit = lsb + index_shift; bit < msb; ++bit) {
		res.arr[bit] = this->arr[bit - index_shift];
	}
	unsigned char bit_shift = (unsigned char)(shift_count - index_shift * 64);
	word low = 0;
	for (index bit = lsb; bit < msb; bit++) {
		word curr_bit = res.arr[bit];
		res.arr[bit] = _shiftleft128(low, curr_bit, bit_shift);
		low = curr_bit;
	}
	return res;
}

BigNum BigNum::operator>>(word shift_count) const
{
	index index_shift = shift_count / 64;
	if (index_shift > msb - lsb) throw std::exception("Shift count too large");
	BigNum res(msb, lsb);
	for (index bit = lsb; bit < msb - index_shift; ++bit) {
		res.arr[bit] = this->arr[bit + index_shift];
		std::cout << res.arr[bit];
	}
	for (index bit = msb - shift_count / 64; bit < msb; ++bit) {
		res.arr[bit] = 0;
	}
	unsigned char bit_shift = (unsigned char)(shift_count - index_shift * 64);
	word high = 0;
	for (index bit = msb - 1; bit >= lsb; bit--) {
		word curr_bit = res.arr[bit];
		res.arr[bit] = _shiftright128(curr_bit, high, bit_shift);
		high = curr_bit;
	}
	return res;
}

BigNum& BigNum::operator<<=(word shift_count)
{
	index index_shift = shift_count / 64;
	if (index_shift > msb - lsb) throw std::exception("Shift count too large");
	for (index bit = msb - 1; bit >= lsb + index_shift; --bit) {
		this->arr[bit] = this->arr[bit - index_shift];
	}
	for (index bit = lsb; bit < lsb + index_shift; ++bit) {
		this->arr[bit] = 0;
	}
	unsigned char bit_shift = (unsigned char)(shift_count - index_shift * 64);
	word low = 0;
	for (index bit = lsb; bit < msb; bit++) {
		word curr_bit = this->arr[bit];
		this->arr[bit] = _shiftleft128(low, curr_bit, bit_shift);
		low = curr_bit;
	}
	return *this;
}

BigNum& BigNum::operator>>=(word shift_count)
{
	index index_shift = shift_count / 64;
	if (index_shift > msb - lsb) throw std::exception("Shift count too large");
	for (index bit = lsb; bit < msb - index_shift; ++bit) {
		this->arr[bit] = this->arr[bit + index_shift];
	}
	for (index bit = msb - index_shift; bit < msb; ++bit) {
		this->arr[bit] = 0;
	}
	unsigned char bit_shift = (unsigned char)(shift_count - index_shift * 64);
	word high = 0;
	for (index bit = msb - 1; bit >= lsb; bit--) {
		word curr_bit = this->arr[bit];
		this->arr[bit] = _shiftright128(curr_bit, high, bit_shift);
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

std::string BigNum::toDecimalString()
{
	std::stringstream ss;
	ss << std::dec << *this;
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

BigNum invsqrt(const BigNum& x)
{
	BigNum r = BigNum(0ull, x.msb, x.lsb);
	r.arr[-1] = 184421341163531203;
	BigNum one = BigNum(1ull, x.msb, x.lsb);
	BigNum three = BigNum(3ull, x.msb, x.lsb);

	BigNum rPrev = BigNum(0ull, x.msb, x.lsb);

	while (1) {
		if (r == rPrev) {
			break;
		}
		BigNum r2 = r * r;
		BigNum r2x = r2 * x;
		index prec = get_msb((((three - r2x) / 2ll) - one).arr, x.msb, x.lsb);
		if (prec <= x.lsb / 2) break;
		rPrev = r;
		r *= ((three - r2x) / 2ll);
	}

	return r;
}