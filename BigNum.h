#pragma once
#include <iostream>
#include <memory>
#include <string>

using word = unsigned long long;
using index = long long;
using int64 = long long;

class BigNum
{
public:
	BigNum(index msb, index lsb);
	BigNum(int64 w, index msb, index lsb);
	BigNum(const BigNum& other, index msb, index lsb);

	friend BigNum add(const BigNum& n1, const BigNum& n2, bool neg);
	friend BigNum sub(const BigNum& n1, const BigNum& n2, bool neg);

	BigNum operator+(const BigNum& other) const;
	BigNum operator-(const BigNum& other) const;
	BigNum& operator+=(const BigNum& other);
	BigNum& operator-=(const BigNum& other);

	BigNum operator*(const BigNum& other) const;
	BigNum operator*(int64 other) const;
	BigNum& operator*=(const BigNum& other);
	BigNum& operator*=(int64 other);

	BigNum operator/(const BigNum& other) const;
	BigNum operator/(int64 other) const;
	BigNum& operator/=(const BigNum& other);
	BigNum& operator/=(int64 other);

	BigNum operator%(const BigNum& other) const;
	word operator%(word other) const;

	BigNum& operator++(); //prefix
	BigNum operator++(int); //postfix
	BigNum& operator--(); //prefix
	BigNum operator--(int); //postfix

	BigNum& operator=(int64 w);
	BigNum& operator=(const BigNum& other);

	BigNum operator&(const BigNum& other) const;
	BigNum operator|(const BigNum& other) const;
	BigNum operator^(const BigNum& other) const;
	BigNum& operator&=(const BigNum& other);
	BigNum& operator|=(const BigNum& other);
	BigNum& operator^=(const BigNum& other);
	BigNum operator~() const;

	BigNum operator<<(word shift_count) const;
	BigNum operator>>(word shift_count) const;
	BigNum& operator<<=(word shift_count);
	BigNum& operator>>=(word shift_count);

	inline bool operator&&(const BigNum& other) const { return bool(*this) && bool(other); }
	inline bool operator||(const BigNum& other) const { return bool(*this) || bool(other); }
	inline bool operator!() const { return !bool(*this); }

	operator bool() const;

	friend int compare(const BigNum& r1, const BigNum& r2);
	inline bool operator<(const BigNum& other) const { return compare(*this, other) < 0; }
	inline bool operator>(const BigNum& other) const { return compare(*this, other) > 0; }
	inline bool operator<=(const BigNum& other) const { return compare(*this, other) <= 0; }
	inline bool operator>=(const BigNum& other) const { return compare(*this, other) >= 0; }
	inline bool operator==(const BigNum& other) const { return compare(*this, other) == 0; }
	inline bool operator!=(const BigNum& other) const { return compare(*this, other) != 0; }

	friend std::ostream& operator<<(std::ostream& os, const BigNum& ref);

	friend BigNum invsqrt(const BigNum& x);

	std::string toHexString();

private:
	std::shared_ptr<word[]> ptr;
	word* arr;
	bool negative;
	const index msb;
	const index lsb;
};